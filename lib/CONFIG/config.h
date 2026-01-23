#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <stdint.h>

/*
## Pinout ##
| ESP32 | RX5880 |
| :------------- |:-------------|
| 33 | RSSI |
| GND | GND |
| 19 | CH1 |
| 22 | CH2 |
| 23 | CH3 |
| 3V3 | +5V |

* **Led** goes to pin 21 and GND
* The optional **Buzzer** goes to pin 25 or 27 and GND

*/

//ESP32-C3
#if defined(ESP32C3)

#define PIN_LED 1
#define PIN_VBAT 0
#define VBAT_SCALE 2
#define VBAT_ADD 2
#define PIN_RX5808_RSSI 3
#define PIN_RX5808_DATA 6     //CH1
#define PIN_RX5808_SELECT 7   //CH2
#define PIN_RX5808_CLOCK 4    //CH3
#define PIN_BUZZER 5
#define BUZZER_INVERTED false
#define PIN_MODE_SWITCH 1     // Mode selection: LOW=WiFi, HIGH=RotorHazard
// SD Card SPI pins
#define PIN_SD_CS 8
#define PIN_SD_SCK 2
#define PIN_SD_MOSI 10
#define PIN_SD_MISO 9

//ESP32-S3 Super Mini
#elif defined(ESP32S3_SUPERMINI)

#define PIN_LED 2              // External status LED (if present)
#define PIN_RGB_LED 1          // WS2812 RGB LED on GPIO1
#define PIN_VBAT 0
#define VBAT_SCALE 2
#define VBAT_ADD 2
#define PIN_RX5808_RSSI 3      // RSSI on Pin 3
#define PIN_RX5808_DATA 6      // CH1 on Pin 6
#define PIN_RX5808_SELECT 7    // CH2 on Pin 7
#define PIN_RX5808_CLOCK 4     // CH3 on Pin 4
#define PIN_BUZZER 5
#define BUZZER_INVERTED false
#define PIN_MODE_SWITCH 1      // Mode selection: LOW=WiFi, HIGH=RotorHazard
// SD Card SPI pins
#define PIN_SD_CS 8
#define PIN_SD_SCK 2
#define PIN_SD_MOSI 10
#define PIN_SD_MISO 9

//LilyGO T-Energy S3
#elif defined(LILYGO_TENERGY_S3)

#define PIN_LED 2              // SAME AS DEVKITC: External LED on GPIO 2
#define PIN_RGB_LED 48         // SAME AS DEVKITC: WS2812 RGB LED
#define PIN_VBAT 3             // DIFFERENT: Battery voltage sense (T-Energy built-in divider on GPIO 3)
#define VBAT_SCALE 2           // 2:1 voltage divider (100K/100K)
#define VBAT_ADD 2             // Calibration offset
#define PIN_RX5808_RSSI 4      // SAME AS DEVKITC: RSSI on GPIO 4
#define PIN_RX5808_DATA 10     // SAME AS DEVKITC: CH1 on Pin 10
#define PIN_RX5808_SELECT 11   // SAME AS DEVKITC: CH2 on Pin 11
#define PIN_RX5808_CLOCK 12    // SAME AS DEVKITC: CH3 on Pin 12
#define PIN_BUZZER 5           // SAME AS DEVKITC: Buzzer on Pin 5
#define BUZZER_INVERTED false
#define PIN_MODE_SWITCH 9      // SAME AS DEVKITC: Mode selection
// SD Card SPI pins (SAME AS DEVKITC)
#define PIN_SD_CS 39           // SAME AS DEVKITC
#define PIN_SD_SCK 36          // SAME AS DEVKITC
#define PIN_SD_MOSI 35         // SAME AS DEVKITC
#define PIN_SD_MISO 37         // SAME AS DEVKITC

// Seeed Studio XIAO ESP32S3
#elif defined(SEEED_XIAO_ESP32S3)

#define PIN_LED 21             // Onboard user LED
#define PIN_RGB_LED 44         // NeoPixel signal on D7 (GPIO44)
#define PIN_VBAT 0             // External divider required if used
#define VBAT_SCALE 2
#define VBAT_ADD 2
#define PIN_RX5808_RSSI 3      // D2
#define PIN_RX5808_DATA 5      // D4
#define PIN_RX5808_SELECT 6    // D5
#define PIN_RX5808_CLOCK 4     // D3
#define PIN_BUZZER 5           // D4
#define BUZZER_INVERTED false
#define PIN_MODE_SWITCH 1      // D0
// SD Card SPI pins (XIAO S3 per user wiring)
#define PIN_SD_CS 2            // D1 (CS)
#define PIN_SD_SCK 7           // D8 (SCK)
#define PIN_SD_MOSI 8          // D9 (MOSI)
#define PIN_SD_MISO 9          // D10 (MISO)

//ESP32-S3 DevKitC
#elif defined(ESP32S3)

#define PIN_LED 2
#define PIN_RGB_LED 48         // WS2812 RGB LED on ESP32-S3-DevKitC-1
#define PIN_VBAT 1             // Battery voltage test pin (connect 3.0-4.2V via voltage divider)
#define VBAT_SCALE 2           // 2:1 voltage divider (adjust based on your divider)
#define VBAT_ADD 2             // Calibration offset: +0.2V correction
#define PIN_RX5808_RSSI 4      // RSSI on Pin 4 (GPIO3 is a strapping pin - causes boot issues!)
#define PIN_RX5808_DATA 10     // CH1 on Pin 10
#define PIN_RX5808_SELECT 11   // CH2 on Pin 11
#define PIN_RX5808_CLOCK 12    // CH3 on Pin 12
#define PIN_BUZZER 5
#define BUZZER_INVERTED false
#define PIN_MODE_SWITCH 9      // Mode selection: LOW=WiFi, HIGH=RotorHazard
// SD Card SPI pins (tested and working configuration)
#define PIN_SD_CS 39
#define PIN_SD_SCK 36
#define PIN_SD_MOSI 35
#define PIN_SD_MISO 37

//ESP32
#else

#define PIN_LED 21
#define PIN_VBAT 35
#define VBAT_SCALE 2
#define VBAT_ADD 2
#define PIN_RX5808_RSSI 33
#define PIN_RX5808_DATA 19   //CH1
#define PIN_RX5808_SELECT 22 //CH2
#define PIN_RX5808_CLOCK 23  //CH3
#define PIN_BUZZER 27
#define BUZZER_INVERTED false
#define PIN_MODE_SWITCH 33   // Mode selection: LOW=WiFi, HIGH=RotorHazard

#endif

// ====================================================================
// Centralized Platform Feature Defines
// Add new board defines here to automatically enable features across the codebase
// ====================================================================

// ESP32-S3 family boards (SD card support, SPI, USB CDC)
#if defined(ESP32S3) || defined(ESP32C3) || defined(ESP32S3_SUPERMINI) || defined(LILYGO_TENERGY_S3) || defined(SEEED_XIAO_ESP32S3)
    #define HAS_SD_CARD_SUPPORT 1
    #define HAS_SPI_CLASS 1
#endif

// ESP32-S3 boards with RGB LED
#if defined(ESP32S3) || defined(ESP32S3_SUPERMINI) || defined(LILYGO_TENERGY_S3) || defined(SEEED_XIAO_ESP32S3)
    #define HAS_RGB_LED 1
#endif

// Boards with built-in battery monitoring
#if defined(LILYGO_TENERGY_S3) || defined(ENABLE_BATTERY_TEST)
    #define HAS_BATTERY_MONITOR 1
#endif

// Mode selection constants
#define WIFI_MODE LOW          // GND on switch pin = WiFi/Standalone mode
#define ROTORHAZARD_MODE HIGH  // HIGH (floating/pullup) = RotorHazard node mode

#define EEPROM_RESERVED_SIZE 512
#define CONFIG_MAGIC_MASK (0b11U << 30)
#define CONFIG_MAGIC (0b01U << 30)
#define CONFIG_VERSION 6U

#define EEPROM_CHECK_TIME_MS 1000

typedef struct {
    uint32_t version;
    uint16_t frequency;
    uint8_t minLap;
    uint8_t alarm;
    uint8_t announcerType;
    uint8_t announcerRate;
    uint8_t enterRssi;
    uint8_t exitRssi;
    uint8_t maxLaps;
    uint8_t ledMode;           // 0=off, 1=solid, 2=pulse, 3=rainbow (legacy, kept for migration)
    uint8_t ledBrightness;     // 0-255
    uint32_t ledColor;         // RGB as 0xRRGGBB (solid color)
    uint8_t ledPreset;         // 0-9 (new preset system)
    uint8_t ledSpeed;          // 1-20 (animation speed)
    uint32_t ledFadeColor;     // RGB for COLOR_FADE preset
    uint32_t ledStrobeColor;   // RGB for STROBE preset
    uint8_t ledManualOverride; // Manual override flag (0=off, 1=on)
    uint8_t operationMode;     // 0=WiFi, 1=RotorHazard (software switch)
    uint8_t tracksEnabled;     // Track feature enabled (0=disabled, 1=enabled)
    uint32_t selectedTrackId;  // Currently selected track (0=none)
    uint8_t webhooksEnabled;   // Webhooks enabled (0=disabled, 1=enabled)
    char webhookIPs[10][16];   // Up to 10 webhook IPs (xxx.xxx.xxx.xxx format)
    uint8_t webhookCount;      // Number of configured webhooks
    uint8_t gateLEDsEnabled;   // Gate LEDs feature enabled (0=disabled, 1=enabled)
    uint8_t webhookRaceStart;  // Send /RaceStart webhook (0=disabled, 1=enabled)
    uint8_t webhookRaceStop;   // Send /RaceStop webhook (0=disabled, 1=enabled)
    uint8_t webhookLap;        // Send /Lap webhook (0=disabled, 1=enabled)
    char pilotName[21];
    char pilotCallsign[21];    // Pilot callsign (for announcements)
    char pilotPhonetic[21];    // Phonetic pronunciation
    uint32_t pilotColor;       // Pilot color (0xRRGGBB)
    char theme[21];            // UI theme name
    char selectedVoice[21];    // Voice selection (default, rachel, piper, etc)
    char lapFormat[11];        // Lap announcement format (full, laptime, timeonly)
    char ssid[33];
    char password[33];
} laptimer_config_t;

class Storage;  // Forward declaration
class BatteryMonitor;  // Forward declaration

class Config {
   public:
    void init();
    void load();
    void write();
    void toJson(AsyncResponseStream& destination, BatteryMonitor* batteryMonitor = nullptr);
    void toJsonString(char* buf);
    void fromJson(JsonObject source);
    void handleEeprom(uint32_t currentTimeMs);
    
    // SD card backup/restore
    void setStorage(Storage* stor) { storage = stor; }
    bool saveToSD();
    bool loadFromSD();

    // getters and setters
    uint16_t getFrequency();
    uint32_t getMinLapMs();
    uint8_t getAlarmThreshold();
    uint8_t getEnterRssi();
    uint8_t getExitRssi();
    uint8_t getMaxLaps();
    uint8_t getLedMode();
    uint8_t getLedBrightness();
    uint32_t getLedColor();
    uint8_t getLedPreset();
    uint8_t getLedSpeed();
    uint32_t getLedFadeColor();
    uint32_t getLedStrobeColor();
    uint8_t getLedManualOverride();
    uint8_t getTracksEnabled();
    uint32_t getSelectedTrackId();
    uint8_t getWebhooksEnabled();
    uint8_t getWebhookCount();
    const char* getWebhookIP(uint8_t index);
    uint8_t getGateLEDsEnabled();
    uint8_t getWebhookRaceStart();
    uint8_t getWebhookRaceStop();
    uint8_t getWebhookLap();
    char* getSsid();
    char* getPassword();
    uint8_t getOperationMode();
    char* getPilotCallsign();
    char* getPilotPhonetic();
    uint32_t getPilotColor();
    char* getTheme();
    char* getSelectedVoice();
    char* getLapFormat();
    
    // Setters for RotorHazard node mode
    void setFrequency(uint16_t freq);
    void setEnterRssi(uint8_t rssi);
    void setExitRssi(uint8_t rssi);
    void setOperationMode(uint8_t mode);
    
    // LED setters
    void setLedPreset(uint8_t preset);
    void setLedBrightness(uint8_t brightness);
    void setLedSpeed(uint8_t speed);
    void setLedColor(uint32_t color);
    void setLedFadeColor(uint32_t color);
    void setLedStrobeColor(uint32_t color);
    void setLedManualOverride(uint8_t override);
    void setTracksEnabled(uint8_t enabled);
    void setSelectedTrackId(uint32_t trackId);
    void setWebhooksEnabled(uint8_t enabled);
    bool addWebhookIP(const char* ip);
    bool removeWebhookIP(const char* ip);
    void clearWebhookIPs();
    void setGateLEDsEnabled(uint8_t enabled);
    void setWebhookRaceStart(uint8_t enabled);
    void setWebhookRaceStop(uint8_t enabled);
    void setWebhookLap(uint8_t enabled);

   private:
    laptimer_config_t conf;
    bool modified;
    volatile uint32_t checkTimeMs = 0;
    Storage* storage = nullptr;
    void setDefaults();
};

#endif // CONFIG_H
