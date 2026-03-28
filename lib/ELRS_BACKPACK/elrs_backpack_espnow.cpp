#include "elrs_backpack_espnow.h"
#include "config.h"
#include "debug.h"
#include "elrs_bind_uid.h"
#include "msp.h"

#if defined(ENABLE_ELRS_BACKPACK_ESPNOW)

#include <cstdio>
#include <cstring>
#include <esp_idf_version.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include <WiFi.h>

static bool s_espnow_ready = false;
static elrs_msp::Decoder s_msp_decoder;

/** After ESP-NOW start, send a few link-test OSD bursts (radio/interface may not be ready on first ms). */
static uint8_t s_link_ping_rounds_left = 0;
static uint32_t s_link_ping_next_ms = 0;

/** From esp_now send callback: SUCCESS means link-layer ack from peer (unicast); broadcast often reports FAIL. */
static uint32_t s_espnow_deliver_ok = 0;
static uint32_t s_espnow_deliver_fail = 0;

static void handle_msp_packet(const elrs_msp::Packet& p) {
    DEBUG("[ELRS MSP] type=%c fn=0x%04X len=%u\n", static_cast<char>(p.type), p.function,
          static_cast<unsigned>(p.payload_len));
    (void)p;
}

static void on_recv(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
    (void)info;
    if (!data || len <= 0) {
        return;
    }
    elrs_msp::Packet pkt{};
    for (int i = 0; i < len; i++) {
        if (s_msp_decoder.pushByte(data[i], &pkt)) {
            handle_msp_packet(pkt);
        }
    }
}

static void on_espnow_sent_count(const uint8_t* mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        s_espnow_deliver_ok++;
        return;
    }
    s_espnow_deliver_fail++;
    if (mac_addr) {
        DEBUG("ELRS: esp_now delivery FAIL -> %02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr[0], mac_addr[1],
              mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    } else {
        DEBUG("ELRS: esp_now delivery FAIL (no dest in tx_info)\n");
    }
}

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
static void on_espnow_sent(const esp_now_send_info_t* tx_info, esp_now_send_status_t status) {
    const uint8_t* mac = (tx_info && tx_info->des_addr) ? tx_info->des_addr : nullptr;
    on_espnow_sent_count(mac, status);
}
#else
static void on_espnow_sent(const uint8_t* mac_addr, esp_now_send_status_t status) {
    on_espnow_sent_count(mac_addr, status);
}
#endif

static const uint8_t kEspnowBroadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/** Same as ExpressLRS Backpack Timer_main / Vrx_main SetSoftMACAddress() — LR is required for reliable ESP-NOW to VRx. */
static void apply_elrs_backpack_wifi_protocol(wifi_interface_t ifx) {
    const uint8_t mask =
        WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR;
    esp_err_t e = esp_wifi_set_protocol(ifx, mask);
    if (e != ESP_OK) {
        DEBUG("ELRS: esp_wifi_set_protocol(if=%d) err=%d\n", static_cast<int>(ifx), static_cast<int>(e));
    }
}

/** Apply LR-capable PHY on every interface we might use (AP_STA builds have both). */
static void apply_elrs_backpack_wifi_protocol_for_mode() {
    apply_elrs_backpack_wifi_protocol(WIFI_IF_AP);
    const wifi_mode_t wm = WiFi.getMode();
    if (wm == WIFI_MODE_APSTA || wm == WIFI_MODE_STA) {
        apply_elrs_backpack_wifi_protocol(WIFI_IF_STA);
    }
}

static void espnow_send_uid_and_broadcast(const uint8_t uid[6], const uint8_t* frame, size_t len) {
    if (elrs_backpack_uid_valid(uid)) {
        esp_err_t e = esp_now_send(uid, frame, len);
        if (e != ESP_OK) {
            DEBUG("ELRS: esp_now_send (uid peer) err=%d\n", (int)e);
        }
    }
    esp_err_t eb = esp_now_send(kEspnowBroadcast, frame, len);
    if (eb != ESP_OK) {
        DEBUG("ELRS: esp_now_send (bcast) err=%d\n", (int)eb);
    }
}

/** MSP_ELRS_SET_SEND_UID payload 0x01 + 6-byte UID (vrxc_elrs elrs_backpack.set_send_uid). */
static void send_msp_set_send_uid(const uint8_t uid[6]) {
    uint8_t payload[7];
    payload[0] = 0x01;
    memcpy(&payload[1], uid, 6);
    uint8_t out[256];
    size_t olen = elrs_msp::buildPacketV2(elrs_msp::PacketType::Command, 0,
                                          static_cast<uint16_t>(elrs_msp::MspElrsSetSendUid), payload, 7, out,
                                          sizeof(out));
    if (olen == 0) {
        DEBUG("ELRS: MSP SET_SEND_UID build failed\n");
        return;
    }
    espnow_send_uid_and_broadcast(uid, out, olen);
}

/** MSP_ELRS_SET_OSD sub 0x03 — same layout as vrxc_elrs send_osd_text. Returns payload length (4 + text). */
static size_t build_osd_text_subpayload(uint8_t row, uint8_t col, const char* text, uint8_t* payload,
                                        size_t cap) {
    if (cap < 4 + 50) {
        return 0;
    }
    payload[0] = 0x03;
    payload[1] = row;
    payload[2] = col;
    payload[3] = 0;
    size_t n = 0;
    for (; text[n] && n < 50; ++n) {
        payload[4 + n] = static_cast<uint8_t>(text[n]);
    }
    return 4 + n;
}

static void send_msp_set_osd_payload(const uint8_t uid[6], const uint8_t* osd_payload, uint16_t osd_len) {
    uint8_t out[256];
    size_t olen = elrs_msp::buildPacketV2(elrs_msp::PacketType::Command, 0,
                                          static_cast<uint16_t>(elrs_msp::MspElrsSetOsd), osd_payload, osd_len, out,
                                          sizeof(out));
    if (olen == 0) {
        DEBUG("ELRS: MSP SET_OSD build failed (len=%u)\n", static_cast<unsigned>(osd_len));
        return;
    }
    espnow_send_uid_and_broadcast(uid, out, olen);
}

static constexpr unsigned kHdzeroOsdCols = 50;

static void elrs_send_text_row_then_display(Config* conf, uint8_t row, const char* text) {
    if (!conf || !text) {
        return;
    }
    uint8_t uid[6]{};
    elrs_backpack_compute_uid6(conf->getElrsBackpackBindPhrase(), conf->getPilotCallsign(), uid);
    send_msp_set_send_uid(uid);

    const uint8_t cfgCol = conf->getElrsOsdLapCol();
    const size_t text_len = strnlen(text, 56);
    uint8_t col = 0;
    if (cfgCol == ELRS_OSD_LAP_COL_AUTO) {
        if (text_len < kHdzeroOsdCols) {
            col = static_cast<uint8_t>((kHdzeroOsdCols - text_len) / 2U);
        }
    } else {
        col = cfgCol;
    }

    uint8_t pl[4 + 50]{};
    size_t plen = build_osd_text_subpayload(row, col, text, pl, sizeof(pl));
    if (plen == 0) {
        return;
    }
    send_msp_set_osd_payload(uid, pl, static_cast<uint16_t>(plen));
    const uint8_t display_sub[1] = {0x04};
    send_msp_set_osd_payload(uid, display_sub, 1);
}

static void elrs_send_text_then_display(Config* conf, const char* text) {
    elrs_send_text_row_then_display(conf, conf->getElrsOsdLapRow(), text);
}

/** Two lines: row 0 (often visible for backpack OSD) + configured lap row. */
static void elrs_send_link_ping(Config* conf) {
    elrs_send_text_row_then_display(conf, 0, "FPVGate");
    elrs_send_text_row_then_display(conf, conf->getElrsOsdLapRow(), "Timer link OK");
}

void elrsBackpackEspnowOnLap(Config* conf, uint32_t lapTimeMs, uint16_t lapNumberCompleted) {
    if (!conf || !conf->getElrsBackpackEspnow() || !s_espnow_ready) {
        return;
    }

    const uint32_t sec = lapTimeMs / 1000U;
    const uint32_t tenth = (lapTimeMs % 1000U) / 100U;
    const uint32_t m = sec / 60U;
    const uint32_t s = sec % 60U;

    char text[56];
    snprintf(text, sizeof(text), "LAP %u %lu:%02lu.%lu", static_cast<unsigned>(lapNumberCompleted),
             static_cast<unsigned long>(m), static_cast<unsigned long>(s), static_cast<unsigned long>(tenth));
    elrs_send_text_then_display(conf, text);
}

void elrsBackpackEspnowOnPlaybackLap(Config* conf, uint32_t lapTimeMs) {
    if (!conf || !conf->getElrsBackpackEspnow() || !conf->getElrsOsdPlaybackLaps() || !s_espnow_ready) {
        return;
    }
    const uint32_t sec = lapTimeMs / 1000U;
    const uint32_t tenth = (lapTimeMs % 1000U) / 100U;
    const uint32_t m = sec / 60U;
    const uint32_t s = sec % 60U;
    char text[56];
    snprintf(text, sizeof(text), "PB %lu:%02lu.%lu", static_cast<unsigned long>(m),
             static_cast<unsigned long>(s), static_cast<unsigned long>(tenth));
    elrs_send_text_then_display(conf, text);
}

void elrsBackpackEspnowOnRaceStop(Config* conf) {
    if (!conf || !conf->getElrsBackpackEspnow() || !conf->getElrsOsdClearOnStop() || !s_espnow_ready) {
        return;
    }
    uint8_t uid[6]{};
    elrs_backpack_compute_uid6(conf->getElrsBackpackBindPhrase(), conf->getPilotCallsign(), uid);
    send_msp_set_send_uid(uid);
    const uint8_t clear_sub[1] = {0x02};
    send_msp_set_osd_payload(uid, clear_sub, 1);
    const uint8_t display_sub[1] = {0x04};
    send_msp_set_osd_payload(uid, display_sub, 1);
}

static void add_or_update_peer(const uint8_t mac[6], wifi_interface_t ifidx) {
    esp_now_del_peer(mac);
    esp_now_peer_info_t peer{};
    memcpy(peer.peer_addr, mac, 6);
    peer.channel = 0;
    peer.encrypt = false;
    peer.ifidx = ifidx;
    esp_err_t err = esp_now_add_peer(&peer);
    if (err != ESP_OK && err != ESP_ERR_ESPNOW_EXIST) {
        DEBUG("ELRS: esp_now_add_peer %02x:... err=%d\n", mac[0], (int)err);
    }
}

void elrsBackpackEspnowPrepareApMacAsUid(Config* conf) {
    if (!conf || !conf->getElrsBackpackEspnow()) {
        return;
    }
    uint8_t uid[6]{};
    elrs_backpack_compute_uid6(conf->getElrsBackpackBindPhrase(), conf->getPilotCallsign(), uid);
    if (!elrs_backpack_uid_valid(uid)) {
        return;
    }
    esp_err_t e = esp_wifi_set_mac(WIFI_IF_AP, uid);
    if (e != ESP_OK) {
        DEBUG("ELRS: esp_wifi_set_mac(WIFI_IF_AP, UID) err=%d (VRx requires sender MAC == UID)\n", (int)e);
    } else {
        DEBUG("ELRS: softAP MAC set to bind UID (VRx ESP-NOW sender check)\n");
    }
}

void elrsBackpackEspnowStopIfRunning() {
    s_link_ping_rounds_left = 0;
    if (!s_espnow_ready) {
        return;
    }
    s_msp_decoder.reset();
    esp_now_unregister_recv_cb();
    esp_now_unregister_send_cb();
    esp_now_deinit();
    s_espnow_ready = false;
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    DEBUG("ELRS backpack ESP-NOW stopped\n");
}

void elrsBackpackEspnowStartIfEnabled(Config* conf) {
    if (!conf || !conf->getElrsBackpackEspnow()) {
        elrsBackpackEspnowStopIfRunning();
        return;
    }

    elrsBackpackEspnowStopIfRunning();

    esp_err_t err = esp_now_init();
    if (err != ESP_OK) {
        DEBUG("esp_now_init failed: %d\n", (int)err);
        return;
    }

    err = esp_now_register_recv_cb(on_recv);
    if (err != ESP_OK) {
        DEBUG("esp_now_register_recv_cb failed: %d\n", (int)err);
        esp_now_deinit();
        return;
    }

    err = esp_now_register_send_cb(on_espnow_sent);
    if (err != ESP_OK) {
        DEBUG("esp_now_register_send_cb failed: %d\n", (int)err);
        esp_now_unregister_recv_cb();
        esp_now_deinit();
        return;
    }

    s_espnow_deliver_ok = 0;
    s_espnow_deliver_fail = 0;

    esp_wifi_set_ps(WIFI_PS_NONE);

    apply_elrs_backpack_wifi_protocol_for_mode();

    /* Default STA caused ESP-NOW to attach to STA while AP-only (no home WiFi): packets never left the AP path. */
    wifi_interface_t ifidx = WIFI_IF_AP;
    if (WiFi.status() == WL_CONNECTED) {
        ifidx = WIFI_IF_STA;
    }

    add_or_update_peer(kEspnowBroadcast, ifidx);

    uint8_t uid[6]{};
    elrs_backpack_compute_uid6(conf->getElrsBackpackBindPhrase(), conf->getPilotCallsign(), uid);
    if (elrs_backpack_uid_valid(uid)) {
        add_or_update_peer(uid, ifidx);
        DEBUG("ELRS: backpack UID %02X:%02X:%02X:%02X:%02X:%02X (bind phrase or callsign hash)\n", uid[0], uid[1],
              uid[2], uid[3], uid[4], uid[5]);
        send_msp_set_send_uid(uid);
    } else {
        DEBUG("ELRS: no bind phrase or callsign — broadcast peer only; set phrase to match goggles\n");
    }

    s_espnow_ready = true;
    {
        uint8_t ch = 0;
        wifi_second_chan_t second = WIFI_SECOND_CHAN_NONE;
        if (esp_wifi_get_channel(&ch, &second) == ESP_OK) {
            DEBUG("ELRS: Wi-Fi channel %u (peer channel 0 = follow this; match goggle RX band)\n",
                  static_cast<unsigned>(ch));
        }
    }
    DEBUG("ELRS backpack ESP-NOW started (esp_now if=%d)\n", static_cast<int>(ifidx));

    /* Deferred + repeated: first attempt after Wi-Fi/ESP-NOW often needs a few hundred ms; HDZero may show row 0 vs lap row differently. */
    s_link_ping_rounds_left = 14;
    s_link_ping_next_ms = millis() + 250;
    DEBUG("ELRS: link test: %u OSD bursts (~450ms); serial shows cb ok/fail (bcast FAIL is normal)\n",
          static_cast<unsigned>(s_link_ping_rounds_left));
}

void elrsBackpackEspnowPoll(Config* conf, uint32_t nowMs) {
    if (!s_espnow_ready || !conf || !conf->getElrsBackpackEspnow() || s_link_ping_rounds_left == 0) {
        return;
    }
    if (static_cast<int32_t>(nowMs - s_link_ping_next_ms) < 0) {
        return;
    }
    const uint8_t round = static_cast<uint8_t>(s_link_ping_rounds_left);
    elrs_send_link_ping(conf);
    s_link_ping_rounds_left--;
    s_link_ping_next_ms = nowMs + 450;
    DEBUG("ELRS: link OSD burst #%u left=%u | esp_now_cb ok=%lu fail=%lu (want ok>0 to UID for backpack)\n",
          static_cast<unsigned>(round), static_cast<unsigned>(s_link_ping_rounds_left),
          static_cast<unsigned long>(s_espnow_deliver_ok), static_cast<unsigned long>(s_espnow_deliver_fail));
}

#else

void elrsBackpackEspnowPrepareApMacAsUid(Config*) {}

void elrsBackpackEspnowStopIfRunning() {}

void elrsBackpackEspnowStartIfEnabled(Config*) {}

void elrsBackpackEspnowOnLap(Config*, uint32_t, uint16_t) {}

void elrsBackpackEspnowOnPlaybackLap(Config*, uint32_t) {}

void elrsBackpackEspnowOnRaceStop(Config*) {}

void elrsBackpackEspnowPoll(Config*, uint32_t) {}

#endif
