#ifndef ELRS_BACKPACK_ESPNOW_H
#define ELRS_BACKPACK_ESPNOW_H

#include <stdint.h>

class Config;

/** Tear down ESP-NOW if it was started (safe if already off). */
void elrsBackpackEspnowStopIfRunning();

/**
 * If ELRS backpack ESP-NOW is enabled in config and the build supports it,
 * initialize ESP-NOW after Wi-Fi is started. Otherwise ensures ESP-NOW is off.
 */
void elrsBackpackEspnowStartIfEnabled(Config* conf);

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
