#include "config.h"

#include <EEPROM.h>
#include <LittleFS.h>

#include "debug.h"
#include "storage.h"
#include "battery.h"

#ifdef ESP32S3
#include <SD.h>
#endif

#define CONFIG_BACKUP_PATH "/config_backup.bin"

// Frequency lookup table (same as frontend script.js)
static const uint16_t freqLookup[][8] = {
  {5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725}, // A
  {5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866}, // B
  {5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945}, // E
  {5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880}, // F
  {5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917}, // R (RaceBand)
  {5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621}, // L (LowBand)
  {5660, 5695, 5735, 5770, 5805, 5878, 5914, 5839}, // DJIv1-25
  {5735, 5770, 5805, 0, 0, 0, 0, 5839},             // DJIv1-25CE
  {5695, 5770, 5878, 0, 0, 0, 0, 5839},             // DJIv1_50
  {5669, 5705, 5768, 5804, 5839, 5876, 5912, 0},    // DJI03/04-20
  {5768, 5804, 5839, 0, 0, 0, 0, 0},                // DJI03/04-20CE
  {5677, 5794, 5902, 0, 0, 0, 0, 0},                // DJI03/04-40
  {5794, 0, 0, 0, 0, 0, 0, 0},                      // DJI03/04-40CE
  {5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917}, // DJI04-R
  {5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917}, // HDZero-R
  {5707, 0, 0, 0, 0, 0, 0, 0},                      // HDZero-E
  {5740, 5760, 0, 5800, 0, 0, 0, 0},                // HDZero-F
  {5732, 5769, 5806, 5843, 0, 0, 0, 0},             // HDZero-CE
  {5658, 5659, 5732, 5769, 5806, 5843, 5880, 5917}, // WalkSnail-R
  {5660, 5695, 5735, 5770, 5805, 5878, 5914, 5839}, // WalkSnail-25
  {5735, 5770, 5805, 0, 0, 0, 0, 5839},             // WalkSnail-25CE
  {5695, 5770, 5878, 0, 0, 0, 0, 5839},             // WalkSnail-50
};

// Get band and channel indices from frequency
// Returns the FIRST match to prioritize analog bands over digital
static void getBandChannelFromFreq(uint16_t freq, int8_t* bandIndex, int8_t* channelIndex) {
    *bandIndex = -1;
    *channelIndex = -1;
    
    // Search through all bands (iterate forwards to prioritize analog bands A, B, E, F, R, L)
    for (int i = 0; i < 22; i++) {
        for (int j = 0; j < 8; j++) {
            if (freqLookup[i][j] == freq) {
                *bandIndex = i;
                *channelIndex = j;
                return; // Return first match (analog bands come first)
            }
        }
    }
}

void Config::init(void) {
    if (sizeof(laptimer_config_t) > EEPROM_RESERVED_SIZE) {
        DEBUG("Config size too big, adjust reserved EEPROM size\n");
        return;
    }

    EEPROM.begin(EEPROM_RESERVED_SIZE);  // Size of EEPROM
    load();                              // Override default settings from EEPROM

    checkTimeMs = millis();

    DEBUG("EEPROM Init Successful\n");
}

void Config::load(void) {
    modified = false;
    EEPROM.get(0, conf);

    uint32_t version = 0xFFFFFFFF;
    if ((conf.version & CONFIG_MAGIC_MASK) == CONFIG_MAGIC) {
        version = conf.version & ~CONFIG_MAGIC_MASK;
    }
    
    DEBUG("EEPROM load: version=0x%08X, timerNumber=%d\n", conf.version, conf.timerNumber);

    // If version is not current, try to restore from SD backup
    if (version != CONFIG_VERSION) {
        DEBUG("EEPROM config invalid (version=%u, expected=%u)\n", version, CONFIG_VERSION);
        if (loadFromSD()) {
            DEBUG("Successfully restored config from SD card backup\n");
            DEBUG("After SD restore: timerNumber=%d\n", conf.timerNumber);
            modified = true;  // Mark as modified to write back to EEPROM
            write();  // Write restored config to EEPROM
        } else {
            DEBUG("No SD backup found, using defaults\n");
            setDefaults();
        }
    }

    // Sanity: announcerRate stored as x10 (1–20). If invalid, reset to default (10).
    if (conf.announcerRate < 1 || conf.announcerRate > 20) {
        DEBUG("Invalid announcerRate=%u; resetting to default 10\n", conf.announcerRate);
        conf.announcerRate = 10;
        modified = true;
    }
}

void Config::write(void) {
    if (!modified) return;

    DEBUG("Writing to EEPROM (timerNumber=%d)\n", conf.timerNumber);

    EEPROM.put(0, conf);
    EEPROM.commit();

    DEBUG("Writing to EEPROM done\n");
    
    // Also backup to SD card if available
    if (saveToSD()) {
        DEBUG("Config backed up to SD card\n");
    }

    modified = false;
}

void Config::toJson(AsyncResponseStream& destination, BatteryMonitor* batteryMonitor) {
    // Use https://arduinojson.org/v6/assistant to estimate memory
    DynamicJsonDocument config(768);
    config["freq"] = conf.frequency;
    
    // Use stored band/channel indices if valid, otherwise compute from frequency
    if (conf.bandIndex < 22 && conf.channelIndex < 8) {
        config["bandIndex"] = conf.bandIndex;
        config["channelIndex"] = conf.channelIndex;
    } else {
        // Fallback: compute from frequency
        int8_t bandIdx = -1;
        int8_t channelIdx = -1;
        getBandChannelFromFreq(conf.frequency, &bandIdx, &channelIdx);
        if (bandIdx >= 0 && channelIdx >= 0) {
            config["bandIndex"] = bandIdx;
            config["channelIndex"] = channelIdx;
        }
    }
    
    config["minLap"] = conf.minLap;
    config["alarm"] = conf.alarm;
    config["anType"] = conf.announcerType;
    config["anRate"] = conf.announcerRate;
    config["enterRssi"] = conf.enterRssi;
    config["exitRssi"] = conf.exitRssi;
    config["maxLaps"] = conf.maxLaps;
    config["ledMode"] = conf.ledMode;
    config["ledBrightness"] = conf.ledBrightness;
    config["ledColor"] = conf.ledColor;
    config["ledPreset"] = conf.ledPreset;
    config["ledSpeed"] = conf.ledSpeed;
    config["ledFadeColor"] = conf.ledFadeColor;
    config["ledStrobeColor"] = conf.ledStrobeColor;
    config["ledManualOverride"] = conf.ledManualOverride;
    config["opMode"] = conf.operationMode;
    config["tracksEnabled"] = conf.tracksEnabled;
    config["selectedTrackId"] = conf.selectedTrackId;
    config["webhooksEnabled"] = conf.webhooksEnabled;
    config["webhookCount"] = conf.webhookCount;
    JsonArray webhooks = config.createNestedArray("webhookIPs");
    for (uint8_t i = 0; i < conf.webhookCount; i++) {
        webhooks.add(conf.webhookIPs[i]);
    }
    config["gateLEDsEnabled"] = conf.gateLEDsEnabled;
    config["webhookRaceStart"] = conf.webhookRaceStart;
    config["webhookRaceStop"] = conf.webhookRaceStop;
    config["webhookLap"] = conf.webhookLap;
    config["name"] = conf.pilotName;
    config["pilotCallsign"] = conf.pilotCallsign;
    config["pilotPhonetic"] = conf.pilotPhonetic;
    config["pilotColor"] = conf.pilotColor;
    config["theme"] = conf.theme;
    config["selectedVoice"] = conf.selectedVoice;
    config["lapFormat"] = conf.lapFormat;
    config["ssid"] = conf.ssid;
    config["pwd"] = conf.password;
    config["batteryType"] = conf.batteryType;
    config["batteryCells"] = conf.batteryCells;
    config["lowBatteryAlarmPerCell"] = conf.lowBatteryAlarmPerCell;
    config["batteryAlarmEnabled"] = conf.batteryAlarmEnabled;
    config["batteryVoltageDivider"] = conf.batteryVoltageDivider;
    
    // Race sync settings
    DEBUG("toJson: sending timerNumber=%d\n", conf.timerNumber);
    config["timerNumber"] = conf.timerNumber;
    config["raceSyncMode"] = conf.raceSyncMode;
    config["syncedTimerCount"] = conf.syncedTimerCount;
    JsonArray syncTimers = config.createNestedArray("syncedTimers");
    for (uint8_t i = 0; i < conf.syncedTimerCount; i++) {
        syncTimers.add(conf.syncedTimers[i]);
    }
    
    // Add battery voltage if monitor exists
    if (batteryMonitor) {
        uint16_t batteryVoltage = batteryMonitor->getBatteryVoltage();
        float voltage = batteryVoltage / 10.0f;
        config["batteryVoltage"] = voltage;
    }
    
    serializeJson(config, destination);
}

void Config::toJsonString(char* buf) {
    DynamicJsonDocument config(512);
    config["freq"] = conf.frequency;
    config["minLap"] = conf.minLap;
    config["alarm"] = conf.alarm;
    config["anType"] = conf.announcerType;
    config["anRate"] = conf.announcerRate;
    config["enterRssi"] = conf.enterRssi;
    config["exitRssi"] = conf.exitRssi;
    config["maxLaps"] = conf.maxLaps;
    config["ledMode"] = conf.ledMode;
    config["ledBrightness"] = conf.ledBrightness;
    config["ledColor"] = conf.ledColor;
    config["ledPreset"] = conf.ledPreset;
    config["ledSpeed"] = conf.ledSpeed;
    config["ledFadeColor"] = conf.ledFadeColor;
    config["ledStrobeColor"] = conf.ledStrobeColor;
    config["ledManualOverride"] = conf.ledManualOverride;
    config["opMode"] = conf.operationMode;
    config["tracksEnabled"] = conf.tracksEnabled;
    config["selectedTrackId"] = conf.selectedTrackId;
    config["name"] = conf.pilotName;
    config["ssid"] = conf.ssid;
    config["pwd"] = conf.password;
    serializeJsonPretty(config, buf, 256);
}

void Config::fromJson(JsonObject source) {
    // Ensure version is always set correctly when modifying config
    if ((conf.version & CONFIG_MAGIC_MASK) != CONFIG_MAGIC) {
        conf.version = CONFIG_VERSION | CONFIG_MAGIC;
    }
    
    if (source["freq"] != conf.frequency) {
        conf.frequency = source["freq"];
        modified = true;
    }
    // Store band and channel indices if provided
    if (source.containsKey("bandIndex") && source["bandIndex"] != conf.bandIndex) {
        conf.bandIndex = source["bandIndex"];
        modified = true;
    }
    if (source.containsKey("channelIndex") && source["channelIndex"] != conf.channelIndex) {
        conf.channelIndex = source["channelIndex"];
        modified = true;
    }
    if (source["minLap"] != conf.minLap) {
        conf.minLap = source["minLap"];
        modified = true;
    }
    if (source["alarm"] != conf.alarm) {
        conf.alarm = source["alarm"];
        modified = true;
    }
    if (source["anType"] != conf.announcerType) {
        conf.announcerType = source["anType"];
        modified = true;
    }
    if (source.containsKey("anRate")) {
        int r = source["anRate"].as<int>();

        // UI range is 0.1–2.0, stored as x10 => 1–20
        if (r < 1) r = 10;     // fall back to default 1.0
        if (r > 20) r = 20;

        if ((uint8_t)r != conf.announcerRate) {
            conf.announcerRate = (uint8_t)r;
            modified = true;
        }
    }
    if (source["enterRssi"] != conf.enterRssi) {
        conf.enterRssi = source["enterRssi"];
        modified = true;
    }
    if (source["exitRssi"] != conf.exitRssi) {
        conf.exitRssi = source["exitRssi"];
        modified = true;
    }
    if (source["maxLaps"] != conf.maxLaps) {
        conf.maxLaps = source["maxLaps"];
        modified = true;
    }
    if (source.containsKey("ledMode") && source["ledMode"] != conf.ledMode) {
        conf.ledMode = source["ledMode"];
        modified = true;
    }
    if (source.containsKey("ledBrightness") && source["ledBrightness"] != conf.ledBrightness) {
        conf.ledBrightness = source["ledBrightness"];
        modified = true;
    }
    if (source.containsKey("ledColor") && source["ledColor"] != conf.ledColor) {
        conf.ledColor = source["ledColor"];
        modified = true;
    }
    if (source.containsKey("ledPreset") && source["ledPreset"] != conf.ledPreset) {
        conf.ledPreset = source["ledPreset"];
        modified = true;
    }
    if (source.containsKey("ledSpeed") && source["ledSpeed"] != conf.ledSpeed) {
        conf.ledSpeed = source["ledSpeed"];
        modified = true;
    }
    if (source.containsKey("ledFadeColor") && source["ledFadeColor"] != conf.ledFadeColor) {
        conf.ledFadeColor = source["ledFadeColor"];
        modified = true;
    }
    if (source.containsKey("ledStrobeColor") && source["ledStrobeColor"] != conf.ledStrobeColor) {
        conf.ledStrobeColor = source["ledStrobeColor"];
        modified = true;
    }
    if (source.containsKey("ledManualOverride") && source["ledManualOverride"] != conf.ledManualOverride) {
        conf.ledManualOverride = source["ledManualOverride"];
        modified = true;
    }
    if (source.containsKey("opMode") && source["opMode"] != conf.operationMode) {
        conf.operationMode = source["opMode"];
        modified = true;
    }
    if (source.containsKey("tracksEnabled") && source["tracksEnabled"] != conf.tracksEnabled) {
        conf.tracksEnabled = source["tracksEnabled"];
        modified = true;
    }
    if (source.containsKey("selectedTrackId") && source["selectedTrackId"] != conf.selectedTrackId) {
        conf.selectedTrackId = source["selectedTrackId"];
        modified = true;
    }
    if (source.containsKey("gateLEDsEnabled") && source["gateLEDsEnabled"] != conf.gateLEDsEnabled) {
        conf.gateLEDsEnabled = source["gateLEDsEnabled"];
        modified = true;
    }
    if (source.containsKey("webhookRaceStart") && source["webhookRaceStart"] != conf.webhookRaceStart) {
        conf.webhookRaceStart = source["webhookRaceStart"];
        modified = true;
    }
    if (source.containsKey("webhookRaceStop") && source["webhookRaceStop"] != conf.webhookRaceStop) {
        conf.webhookRaceStop = source["webhookRaceStop"];
        modified = true;
    }
    if (source.containsKey("webhookLap") && source["webhookLap"] != conf.webhookLap) {
        conf.webhookLap = source["webhookLap"];
        modified = true;
    }
    // Webhook IPs and enabled state
    if (source.containsKey("webhooksEnabled") && source["webhooksEnabled"] != conf.webhooksEnabled) {
        conf.webhooksEnabled = source["webhooksEnabled"];
        modified = true;
    }
    if (source.containsKey("webhookIPs")) {
        JsonArray webhookArray = source["webhookIPs"].as<JsonArray>();

        bool changed = false;

        // Compare count
        if (webhookArray.size() != conf.webhookCount) {
            changed = true;
        } else {
            // Compare entries
            uint8_t i = 0;
            for (JsonVariant ip : webhookArray) {
                const char* ipStr = ip.as<const char*>() ? ip.as<const char*>() : "";
                if (strcmp(conf.webhookIPs[i], ipStr) != 0) {
                    changed = true;
                    break;
                }
                i++;
            }
        }

        if (changed) {
            memset(conf.webhookIPs, 0, sizeof(conf.webhookIPs));
            conf.webhookCount = 0;

            for (JsonVariant ip : webhookArray) {
                if (conf.webhookCount < 10) {
                    const char* ipStr = ip.as<const char*>() ? ip.as<const char*>() : "";
                    strlcpy(conf.webhookIPs[conf.webhookCount], ipStr, 16);
                    conf.webhookCount++;
                }
            }
            modified = true;
        }
    }

    if (source.containsKey("name")) {
        const char* v = source["name"] | "";
        if (strcmp(v, conf.pilotName) != 0) {
            strlcpy(conf.pilotName, v, sizeof(conf.pilotName));
            modified = true;
        }
    }
    if (source.containsKey("pilotCallsign") && source["pilotCallsign"] != conf.pilotCallsign) {
        strlcpy(conf.pilotCallsign, source["pilotCallsign"] | "", sizeof(conf.pilotCallsign));
        modified = true;
    }
    if (source.containsKey("pilotPhonetic") && source["pilotPhonetic"] != conf.pilotPhonetic) {
        strlcpy(conf.pilotPhonetic, source["pilotPhonetic"] | "", sizeof(conf.pilotPhonetic));
        modified = true;
    }
    if (source.containsKey("pilotColor") && source["pilotColor"] != conf.pilotColor) {
        conf.pilotColor = source["pilotColor"];
        modified = true;
    }
    if (source.containsKey("theme") && source["theme"] != conf.theme) {
        strlcpy(conf.theme, source["theme"] | "oceanic", sizeof(conf.theme));
        modified = true;
    }
    if (source.containsKey("selectedVoice") && source["selectedVoice"] != conf.selectedVoice) {
        strlcpy(conf.selectedVoice, source["selectedVoice"] | "default", sizeof(conf.selectedVoice));
        modified = true;
    }
    if (source.containsKey("lapFormat") && source["lapFormat"] != conf.lapFormat) {
        strlcpy(conf.lapFormat, source["lapFormat"] | "full", sizeof(conf.lapFormat));
        modified = true;
    }
    if (source.containsKey("ssid")) {
        const char* v = source["ssid"] | "";
        if (strcmp(v, conf.ssid) != 0) {
            strlcpy(conf.ssid, v, sizeof(conf.ssid));
            modified = true;
        }
    }
    if (source.containsKey("pwd")) {
        const char* v = source["pwd"] | "";
        if (strcmp(v, conf.password) != 0) {
            strlcpy(conf.password, v, sizeof(conf.password));
            modified = true;
        }
    }
    if (source.containsKey("batteryType") && source["batteryType"] != conf.batteryType) {
        conf.batteryType = source["batteryType"];
        modified = true;
    }
    if (source.containsKey("batteryCells") && source["batteryCells"] != conf.batteryCells) {
        conf.batteryCells = source["batteryCells"];
        modified = true;
    }
    if (source.containsKey("lowBatteryAlarmPerCell")) {
        float v = source["lowBatteryAlarmPerCell"].as<float>();
        if (conf.lowBatteryAlarmPerCell != v) {
            conf.lowBatteryAlarmPerCell = v;
            modified = true;
        }
    }
    if (source.containsKey("batteryAlarmEnabled") && source["batteryAlarmEnabled"] != conf.batteryAlarmEnabled) {
        conf.batteryAlarmEnabled = source["batteryAlarmEnabled"];
        modified = true;
    }
    if (source.containsKey("batteryVoltageDivider")) {
        float v = source["batteryVoltageDivider"].as<float>();
        if (conf.batteryVoltageDivider != v) {
            conf.batteryVoltageDivider = v;
            modified = true;
        }
    }
    
    // Race sync settings
    if (source.containsKey("timerNumber") && source["timerNumber"] != conf.timerNumber) {
        uint8_t num = source["timerNumber"].as<uint8_t>();
        DEBUG("fromJson: timerNumber changing from %d to %d\n", conf.timerNumber, num);
        if (num <= 8) {
            conf.timerNumber = num;
            modified = true;
        }
    }
    if (source.containsKey("raceSyncMode") && source["raceSyncMode"] != conf.raceSyncMode) {
        uint8_t mode = source["raceSyncMode"].as<uint8_t>();
        if (mode <= 2) {
            conf.raceSyncMode = mode;
            modified = true;
        }
    }
    if (source.containsKey("syncedTimers")) {
        JsonArray syncArray = source["syncedTimers"].as<JsonArray>();
        
        bool changed = false;
        
        // Compare count
        if (syncArray.size() != conf.syncedTimerCount) {
            changed = true;
        } else {
            // Compare entries
            uint8_t i = 0;
            for (JsonVariant timer : syncArray) {
                const char* timerStr = timer.as<const char*>() ? timer.as<const char*>() : "";
                if (strcmp(conf.syncedTimers[i], timerStr) != 0) {
                    changed = true;
                    break;
                }
                i++;
            }
        }
        
        if (changed) {
            memset(conf.syncedTimers, 0, sizeof(conf.syncedTimers));
            conf.syncedTimerCount = 0;
            
            for (JsonVariant timer : syncArray) {
                if (conf.syncedTimerCount < 5) {
                    const char* timerStr = timer.as<const char*>() ? timer.as<const char*>() : "";
                    strlcpy(conf.syncedTimers[conf.syncedTimerCount], timerStr, 32);
                    conf.syncedTimerCount++;
                }
            }
            modified = true;
        }
    }
}

uint16_t Config::getFrequency() {
    // === TEMPORARY HARDCODE FOR RX5808 CH1 PIN ISSUE ===
    // Hardcoded to R1 (5658 MHz) - Raceband Channel 1
    // TODO: Remove this once CH1 pin is fixed and revert to: return conf.frequency;
    // return 5658;
    // === END TEMPORARY HARDCODE ===
    return conf.frequency;
}

uint32_t Config::getMinLapMs() {
    return conf.minLap * 100;
}

uint8_t Config::getAlarmThreshold() {
    return conf.alarm;
}

uint8_t Config::getEnterRssi() {
    return conf.enterRssi;
}

uint8_t Config::getExitRssi() {
    return conf.exitRssi;
}

char* Config::getSsid() {
    return conf.ssid;
}

char* Config::getPassword() {
    return conf.password;
}

uint8_t Config::getMaxLaps() {
    return conf.maxLaps;
}

uint8_t Config::getLedMode() {
    return conf.ledMode;
}

uint8_t Config::getLedBrightness() {
    return conf.ledBrightness;
}

uint32_t Config::getLedColor() {
    return conf.ledColor;
}

uint8_t Config::getLedPreset() {
    return conf.ledPreset;
}

uint8_t Config::getLedSpeed() {
    return conf.ledSpeed;
}

uint32_t Config::getLedFadeColor() {
    return conf.ledFadeColor;
}

uint32_t Config::getLedStrobeColor() {
    return conf.ledStrobeColor;
}

uint8_t Config::getLedManualOverride() {
    return conf.ledManualOverride;
}

uint8_t Config::getOperationMode() {
    return conf.operationMode;
}

uint8_t Config::getTracksEnabled() {
    return conf.tracksEnabled;
}

uint32_t Config::getSelectedTrackId() {
    return conf.selectedTrackId;
}

uint8_t Config::getWebhooksEnabled() {
    return conf.webhooksEnabled;
}

uint8_t Config::getWebhookCount() {
    return conf.webhookCount;
}

const char* Config::getWebhookIP(uint8_t index) {
    if (index < conf.webhookCount) {
        return conf.webhookIPs[index];
    }
    return nullptr;
}

uint8_t Config::getGateLEDsEnabled() {
    return conf.gateLEDsEnabled;
}

uint8_t Config::getWebhookRaceStart() {
    return conf.webhookRaceStart;
}

uint8_t Config::getWebhookRaceStop() {
    return conf.webhookRaceStop;
}

uint8_t Config::getWebhookLap() {
    return conf.webhookLap;
}

char* Config::getPilotCallsign() {
    return conf.pilotCallsign;
}

char* Config::getPilotPhonetic() {
    return conf.pilotPhonetic;
}

uint32_t Config::getPilotColor() {
    return conf.pilotColor;
}

char* Config::getTheme() {
    return conf.theme;
}

char* Config::getSelectedVoice() {
    return conf.selectedVoice;
}

char* Config::getLapFormat() {
    return conf.lapFormat;
}

// Setters for RotorHazard node mode
void Config::setFrequency(uint16_t freq) {
    if (conf.frequency != freq) {
        conf.frequency = freq;
        modified = true;
    }
}

void Config::setEnterRssi(uint8_t rssi) {
    if (conf.enterRssi != rssi) {
        conf.enterRssi = rssi;
        modified = true;
    }
}

void Config::setExitRssi(uint8_t rssi) {
    if (conf.exitRssi != rssi) {
        conf.exitRssi = rssi;
        modified = true;
    }
}

void Config::setOperationMode(uint8_t mode) {
    if (conf.operationMode != mode) {
        conf.operationMode = mode;
        modified = true;
    }
}

void Config::setLedPreset(uint8_t preset) {
    if (conf.ledPreset != preset) {
        conf.ledPreset = preset;
        modified = true;
    }
}

void Config::setLedBrightness(uint8_t brightness) {
    if (conf.ledBrightness != brightness) {
        conf.ledBrightness = brightness;
        modified = true;
    }
}

void Config::setLedSpeed(uint8_t speed) {
    if (conf.ledSpeed != speed) {
        conf.ledSpeed = speed;
        modified = true;
    }
}

void Config::setLedColor(uint32_t color) {
    if (conf.ledColor != color) {
        conf.ledColor = color;
        modified = true;
    }
}

void Config::setLedFadeColor(uint32_t color) {
    if (conf.ledFadeColor != color) {
        conf.ledFadeColor = color;
        modified = true;
    }
}

void Config::setLedStrobeColor(uint32_t color) {
    if (conf.ledStrobeColor != color) {
        conf.ledStrobeColor = color;
        modified = true;
    }
}

void Config::setLedManualOverride(uint8_t override) {
    if (conf.ledManualOverride != override) {
        conf.ledManualOverride = override;
        modified = true;
    }
}

void Config::setTracksEnabled(uint8_t enabled) {
    if (conf.tracksEnabled != enabled) {
        conf.tracksEnabled = enabled;
        modified = true;
    }
}

void Config::setSelectedTrackId(uint32_t trackId) {
    if (conf.selectedTrackId != trackId) {
        conf.selectedTrackId = trackId;
        modified = true;
    }
}

void Config::setWebhooksEnabled(uint8_t enabled) {
    if (conf.webhooksEnabled != enabled) {
        conf.webhooksEnabled = enabled;
        modified = true;
    }
}

bool Config::addWebhookIP(const char* ip) {
    if (conf.webhookCount >= 10) {
        DEBUG("Max webhooks reached\n");
        return false;
    }
    
    // Check if IP already exists
    for (uint8_t i = 0; i < conf.webhookCount; i++) {
        if (strcmp(conf.webhookIPs[i], ip) == 0) {
            DEBUG("Webhook IP already exists\n");
            return false;
        }
    }
    
    strlcpy(conf.webhookIPs[conf.webhookCount], ip, 16);
    conf.webhookCount++;
    modified = true;
    return true;
}

bool Config::removeWebhookIP(const char* ip) {
    for (uint8_t i = 0; i < conf.webhookCount; i++) {
        if (strcmp(conf.webhookIPs[i], ip) == 0) {
            // Shift remaining IPs down
            for (uint8_t j = i; j < conf.webhookCount - 1; j++) {
                strlcpy(conf.webhookIPs[j], conf.webhookIPs[j + 1], 16);
            }
            conf.webhookCount--;
            memset(conf.webhookIPs[conf.webhookCount], 0, 16);  // Clear last entry
            modified = true;
            return true;
        }
    }
    return false;
}

void Config::clearWebhookIPs() {
    memset(conf.webhookIPs, 0, sizeof(conf.webhookIPs));
    conf.webhookCount = 0;
    modified = true;
}

void Config::setGateLEDsEnabled(uint8_t enabled) {
    if (conf.gateLEDsEnabled != enabled) {
        conf.gateLEDsEnabled = enabled;
        modified = true;
    }
}

void Config::setWebhookRaceStart(uint8_t enabled) {
    if (conf.webhookRaceStart != enabled) {
        conf.webhookRaceStart = enabled;
        modified = true;
    }
}

void Config::setWebhookRaceStop(uint8_t enabled) {
    if (conf.webhookRaceStop != enabled) {
        conf.webhookRaceStop = enabled;
        modified = true;
    }
}

void Config::setWebhookLap(uint8_t enabled) {
    if (conf.webhookLap != enabled) {
        conf.webhookLap = enabled;
        modified = true;
    }
}

uint8_t Config::getBatteryType() {
    return conf.batteryType;
}

void Config::setBatteryType(uint8_t type) {
    if (conf.batteryType != type) {
        conf.batteryType = type;
        modified = true;
    }
}

uint8_t Config::getBatteryCells() {
    return conf.batteryCells;
}

void Config::setBatteryCells(uint8_t cells) {
    if (conf.batteryCells != cells) {
        conf.batteryCells = cells;
        modified = true;
    }
}

float Config::getLowBatteryAlarmPerCell() {
    return conf.lowBatteryAlarmPerCell;
}

void Config::setLowBatteryAlarmPerCell(float voltage) {
    if (conf.lowBatteryAlarmPerCell != voltage) {
        conf.lowBatteryAlarmPerCell = voltage;
        modified = true;
    }
}

uint8_t Config::getBatteryAlarmEnabled() {
    return conf.batteryAlarmEnabled;
}

void Config::setBatteryAlarmEnabled(uint8_t enabled) {
    if (conf.batteryAlarmEnabled != enabled) {
        conf.batteryAlarmEnabled = enabled;
        modified = true;
    }
}

float Config::getBatteryVoltageDivider() {
    return conf.batteryVoltageDivider;
}

void Config::setBatteryVoltageDivider(float ratio) {
    if (conf.batteryVoltageDivider != ratio) {
        conf.batteryVoltageDivider = ratio;
        modified = true;
    }
}

// Race sync getters and setters
uint8_t Config::getTimerNumber() {
    return conf.timerNumber;
}

void Config::setTimerNumber(uint8_t number) {
    if (number <= 8 && conf.timerNumber != number) {
        conf.timerNumber = number;
        modified = true;
    }
}

uint8_t Config::getRaceSyncMode() {
    return conf.raceSyncMode;
}

void Config::setRaceSyncMode(uint8_t mode) {
    if (mode <= 2 && conf.raceSyncMode != mode) {
        conf.raceSyncMode = mode;
        modified = true;
    }
}

uint8_t Config::getSyncedTimerCount() {
    return conf.syncedTimerCount;
}

const char* Config::getSyncedTimer(uint8_t index) {
    if (index < conf.syncedTimerCount) {
        return conf.syncedTimers[index];
    }
    return nullptr;
}

bool Config::addSyncedTimer(const char* hostname) {
    if (conf.syncedTimerCount >= 5) {
        DEBUG("Max synced timers reached\n");
        return false;
    }
    
    // Check if hostname already exists
    for (uint8_t i = 0; i < conf.syncedTimerCount; i++) {
        if (strcmp(conf.syncedTimers[i], hostname) == 0) {
            DEBUG("Synced timer already exists\n");
            return false;
        }
    }
    
    strlcpy(conf.syncedTimers[conf.syncedTimerCount], hostname, 32);
    conf.syncedTimerCount++;
    modified = true;
    return true;
}

bool Config::removeSyncedTimer(const char* hostname) {
    for (uint8_t i = 0; i < conf.syncedTimerCount; i++) {
        if (strcmp(conf.syncedTimers[i], hostname) == 0) {
            // Shift remaining timers down
            for (uint8_t j = i; j < conf.syncedTimerCount - 1; j++) {
                strlcpy(conf.syncedTimers[j], conf.syncedTimers[j + 1], 32);
            }
            conf.syncedTimerCount--;
            memset(conf.syncedTimers[conf.syncedTimerCount], 0, 32);  // Clear last entry
            modified = true;
            return true;
        }
    }
    return false;
}

void Config::clearSyncedTimers() {
    memset(conf.syncedTimers, 0, sizeof(conf.syncedTimers));
    conf.syncedTimerCount = 0;
    modified = true;
}

void Config::setDefaults(void) {
    DEBUG("Setting EEPROM defaults\n");
    // Reset everything to 0/false and then just set anything that zero is not appropriate
    memset(&conf, 0, sizeof(conf));
    conf.version = CONFIG_VERSION | CONFIG_MAGIC;
    conf.frequency = 5658;  // RaceBand Channel 1 (R1) - 5658 MHz
    conf.bandIndex = 4;  // RaceBand (R)
    conf.channelIndex = 0;  // Channel 1
    conf.minLap = 20;  // 2.0 seconds
    conf.alarm = 0;  // Alarm disabled
    conf.announcerType = 2;
    conf.announcerRate = 10;
    conf.enterRssi = 72;
    conf.exitRssi = 68;
    conf.maxLaps = 0;
    conf.ledMode = 3;  // Rainbow wave by default (legacy)
    conf.ledBrightness = 120;
    conf.ledColor = 14492325;  // 0xDD00A5 (purple-pink)
    conf.ledPreset = 3;  // Color fade preset by default
    conf.ledSpeed = 5;  // Medium speed
    conf.ledFadeColor = 0x0080FF;  // Blue for fade
    conf.ledStrobeColor = 0xFFFFFF;  // White for strobe
    conf.ledManualOverride = 0;  // Manual override off by default
    conf.operationMode = 0;  // WiFi mode by default
    conf.tracksEnabled = 1;  // Tracks enabled by default
    conf.selectedTrackId = 0;  // No track selected by default (will be set on first track creation)
    conf.webhooksEnabled = 0;  // Webhooks disabled by default (no IPs configured)
    conf.webhookCount = 0;  // No webhooks configured
    memset(conf.webhookIPs, 0, sizeof(conf.webhookIPs));  // Clear all webhook IPs
    conf.gateLEDsEnabled = 1;  // Gate LEDs enabled by default
    conf.webhookRaceStart = 1;  // Race start enabled by default
    conf.webhookRaceStop = 1;  // Race stop enabled by default
    conf.webhookLap = 1;  // Lap enabled by default
    strlcpy(conf.pilotName, "Louis", sizeof(conf.pilotName));  // Default pilot name
    strlcpy(conf.pilotCallsign, "Louis", sizeof(conf.pilotCallsign));  // Default callsign
    strlcpy(conf.pilotPhonetic, "Louie", sizeof(conf.pilotPhonetic));  // Default phonetic
    conf.pilotColor = 0x0080FF;  // Default blue color
    strlcpy(conf.theme, "oceanic", sizeof(conf.theme));  // Default theme
    strlcpy(conf.selectedVoice, "piper", sizeof(conf.selectedVoice));  // Default voice
    strlcpy(conf.lapFormat, "timeonly", sizeof(conf.lapFormat));  // Default lap format
    strlcpy(conf.ssid, "", sizeof(conf.ssid));  // Empty WiFi credentials
    strlcpy(conf.password, "", sizeof(conf.password));  // Empty WiFi credentials
    conf.batteryType = BATTERY_TYPE_LIPO;  // Default to LiPo
    conf.batteryCells = 4;  // Default to 4S
    conf.lowBatteryAlarmPerCell = 3.5f;  // Default 3.5V per cell
    conf.batteryAlarmEnabled = 0;  // Disabled by default
    conf.batteryVoltageDivider = 2.0f;  // Default 2:1 voltage divider
    conf.timerNumber = 0;  // Default fpvgate.local
    conf.raceSyncMode = RACE_SYNC_DISABLED;  // Disabled by default
    conf.syncedTimerCount = 0;  // No synced timers
    memset(conf.syncedTimers, 0, sizeof(conf.syncedTimers));  // Clear synced timers
    modified = true;
    write();
}

void Config::handleEeprom(uint32_t currentTimeMs) {
    if (modified && ((currentTimeMs - checkTimeMs) > EEPROM_CHECK_TIME_MS)) {
        checkTimeMs = currentTimeMs;
        write();
    }
}

bool Config::saveToSD() {
    if (!storage || !storage->isSDAvailable()) {
        return false;
    }
    
    DEBUG("Saving config to SD: %s\n", CONFIG_BACKUP_PATH);
    
#ifdef ESP32S3
    File file = SD.open(CONFIG_BACKUP_PATH, FILE_WRITE);
    if (!file) {
        DEBUG("Failed to open config backup file for writing\n");
        return false;
    }
    
    size_t written = file.write((uint8_t*)&conf, sizeof(laptimer_config_t));
    file.close();
    
    if (written != sizeof(laptimer_config_t)) {
        DEBUG("Failed to write complete config (wrote %d of %d bytes)\n", written, sizeof(laptimer_config_t));
        return false;
    }
    
    DEBUG("Config saved to SD (%d bytes)\n", written);
    return true;
#else
    return false;
#endif
}

bool Config::loadFromSD() {
    if (!storage || !storage->isSDAvailable()) {
        return false;
    }
    
    DEBUG("Attempting to load config from SD: %s\n", CONFIG_BACKUP_PATH);
    
#ifdef ESP32S3
    if (!SD.exists(CONFIG_BACKUP_PATH)) {
        DEBUG("No config backup file found on SD\n");
        return false;
    }
    
    File file = SD.open(CONFIG_BACKUP_PATH, FILE_READ);
    if (!file) {
        DEBUG("Failed to open config backup file for reading\n");
        return false;
    }
    
    size_t fileSize = file.size();
    if (fileSize != sizeof(laptimer_config_t)) {
        DEBUG("Config backup file size mismatch (found %d, expected %d)\n", fileSize, sizeof(laptimer_config_t));
        file.close();
        return false;
    }
    
    laptimer_config_t temp_conf;
    size_t bytesRead = file.read((uint8_t*)&temp_conf, sizeof(laptimer_config_t));
    file.close();
    
    if (bytesRead != sizeof(laptimer_config_t)) {
        DEBUG("Failed to read complete config (read %d of %d bytes)\n", bytesRead, sizeof(laptimer_config_t));
        return false;
    }
    
    // Validate the loaded config
    uint32_t version = 0xFFFFFFFF;
    if ((temp_conf.version & CONFIG_MAGIC_MASK) == CONFIG_MAGIC) {
        version = temp_conf.version & ~CONFIG_MAGIC_MASK;
    }
    
    if (version != CONFIG_VERSION) {
        DEBUG("SD config version mismatch (found %u, expected %u)\n", version, CONFIG_VERSION);
        return false;
    }
    
    // Config is valid, use it
    memcpy(&conf, &temp_conf, sizeof(laptimer_config_t));
    DEBUG("Config loaded from SD successfully\n");
    return true;
#else
    return false;
#endif
}
