#ifndef ELRS_BACKPACK_ESPNOW_H
#define ELRS_BACKPACK_ESPNOW_H

#include <stdint.h>

class Config;

/** Tear down ESP-NOW if it was started (safe if already off). */
void elrsBackpackEspnowStopIfRunning();

/**
 * Call after WiFi.mode() and before WiFi.softAP() or WiFi.begin().
 * ExpressLRS VRx only accepts ESP-NOW frames whose *sender* MAC equals the bind UID
 * (see ExpressLRS/Backpack Vrx_main OnDataRecv); the race timer sets STA MAC to UID.
 * We use WIFI_IF_AP when ESP-NOW tx uses the softAP interface (AP-only / not yet STA-connected).
 */
void elrsBackpackEspnowPrepareApMacAsUid(Config* conf);

/**
 * If ELRS backpack ESP-NOW is enabled in config and the build supports it,
 * initialize ESP-NOW after Wi-Fi is started. Otherwise ensures ESP-NOW is off.
 */
void elrsBackpackEspnowStartIfEnabled(Config* conf);

/** Call from main WiFi loop (e.g. Webserver::handleWebUpdate): retries link-test OSD while armed. */
void elrsBackpackEspnowPoll(Config* conf, uint32_t nowMs);

/**
 * On each completed lap, send MSP_ELRS_SET_SEND_UID (refresh), then SET_OSD text + display
 * to the bind-phrase UID peer and broadcast (same path as vrxc_elrs over ESP-NOW).
 * No-op if backpack ESP-NOW is off, not started, or unsupported in this build.
 */
void elrsBackpackEspnowOnLap(Config* conf, uint32_t lapTimeMs, uint16_t lapNumberCompleted);

/** Playback lap line (prefix "PB"); respects OSD row/column config. */
void elrsBackpackEspnowOnPlaybackLap(Config* conf, uint32_t lapTimeMs);

/** If enabled in config, clear goggle OSD (MSP 0x02 + display) after race stop. */
void elrsBackpackEspnowOnRaceStop(Config* conf);

#endif
