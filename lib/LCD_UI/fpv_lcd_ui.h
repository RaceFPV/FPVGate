#pragma once

#include <lvgl.h>
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "CST820.h"

// LCD UI class matching StarForge display implementation
class FpvLcdUI {
public:
    FpvLcdUI();
    ~FpvLcdUI();
    bool begin();

    // FreeRTOS task entry
    static void uiTask(void* parameter);
    
    // Data update methods
    void updateRSSI(uint8_t rssi);
    void addLap(uint32_t lapTimeMs);  // Thread-safe: called from loop() on core 1
    void clearLaps();                  // Thread-safe: called from loop() on core 1
    
    // Button action polling (read from main loop on core 1)
    bool consumeStartRequest();
    bool consumeStopRequest();
    bool consumeClearRequest();
    
    // System/band/channel button polling (read from main loop on core 1)
    int8_t consumeSystemDelta();   // Returns +1, -1, or 0
    int8_t consumeBandDelta();     // Returns +1, -1, or 0
    int8_t consumeChannelDelta();  // Returns +1, -1, or 0
    
    // Threshold button polling (read from main loop on core 1)
    int8_t consumeEnterDelta();    // Returns +5, -5, or 0
    int8_t consumeExitDelta();     // Returns +5, -5, or 0
    
    // Buzzer beep request (read from main loop on core 1)
    uint16_t consumeBuzzerBeep();  // Returns beep duration in ms, or 0
    
    // Thread-safe display update (called from main loop on core 1)
    void setBandChannelDisplay(uint8_t systemIdx, uint8_t bandIdx, uint8_t channelIdx, uint16_t freqMhz);
    void setThresholdDisplay(uint8_t enterRssi, uint8_t exitRssi);
    
    void updateBandChannel(uint8_t band, uint8_t channel);
    void updateFrequency(uint16_t freq);
    void updateThreshold(uint8_t enter_rssi, uint8_t exit_rssi);
    void updateLapTimes(float* lapTimes, uint8_t lapCount);
    void updateBattery(uint8_t percentage, float voltage);
    void updateStatus(const char* status);

private:
    // Display hardware
#if defined(BOARD_ESP32_S3_TOUCH)
    Arduino_DataBus* bus;
    Arduino_GFX* gfx;
    CST820* touch;
#endif

    // LVGL display buffer and driver
    static lv_disp_draw_buf_t s_drawBuf;
#if defined(BOARD_ESP32_S3_TOUCH)
    static lv_color_t* s_buf;  // Dynamically allocated full-screen buffer
#else
    static lv_color_t s_buf[240 * 60]; // 60-line buffer
#endif
    static lv_disp_drv_t s_dispDrv;
    static lv_indev_drv_t s_indevDrv;

    // Tabview and tabs
    lv_obj_t* tabview;
    lv_obj_t* tab_racing;
    lv_obj_t* tab_pilot;
    lv_obj_t* tab_calib;
    
    // UI objects - main elements
    lv_obj_t* rssi_label;
    lv_obj_t* rssi_chart;
    lv_chart_series_t* rssi_series;
    lv_obj_t* lap_count_label;
    lv_obj_t* status_label;
    lv_obj_t* battery_label;
    lv_obj_t* battery_icon;
    
    // Calibration tab elements
    lv_obj_t* calib_rssi_label;
    lv_obj_t* calib_chart;
    lv_chart_series_t* calib_series;
    lv_obj_t* calib_enter_line;
    lv_obj_t* calib_exit_line;
    lv_obj_t* calib_enter_value;
    lv_obj_t* calib_exit_value;
    
    // Control buttons
    lv_obj_t* start_btn;
    lv_obj_t* stop_btn;
    lv_obj_t* clear_btn;
    lv_obj_t* mode_btn;
    
    // Settings controls
    lv_obj_t* system_label;
    lv_obj_t* band_label;
    lv_obj_t* channel_label;
    lv_obj_t* freq_label;
    lv_obj_t* threshold_label;
    lv_obj_t* threshold_enter_label;
    lv_obj_t* threshold_exit_label;
    lv_obj_t* brightness_slider;
    lv_obj_t* brightness_label;
    
    // Lap times display
    lv_obj_t* lap_times_box;
    lv_obj_t* lap_times_labels[5];
    
    // Countdown overlay
    lv_obj_t* countdown_overlay;
    lv_obj_t* countdown_label;
    bool _countdownActive;
    int _countdownValue;
    uint32_t _countdownStartTime;
    int _lastBeepValue;
    static const uint32_t COUNTDOWN_INTERVAL = 1000;  // 1 second per number
    
    // Finish overlay  
    lv_obj_t* finish_overlay;
    lv_obj_t* finish_label;
    bool _finishActive;
    uint32_t _finishStartTime;
    static const uint32_t FINISH_DISPLAY_DURATION = 2000;  // Show for 2 seconds
    
    // Touch and dimming
    uint32_t _lastTouchTime;
    bool _screenDimmed;
    uint8_t _userBrightness;
    
    // Cross-core RSSI feed (written from loop() on core 1, read by UI task on core 0)
    volatile uint8_t _pendingRssi;
    
    // Cross-core button flags (written from UI task on core 0, read by loop() on core 1)
    volatile bool _startRequested;
    volatile bool _stopRequested;
    volatile bool _clearRequested;
    
    // Cross-core system/band/channel button deltas (written from UI task core 0, read by loop core 1)
    volatile int8_t _systemDelta;
    volatile int8_t _bandDelta;
    volatile int8_t _channelDelta;
    
    // Cross-core threshold button deltas (written from UI task core 0, read by loop core 1)
    volatile int8_t _enterDelta;
    volatile int8_t _exitDelta;
    
    // Cross-core band/channel display update (written from loop core 1, read by UI task core 0)
    volatile uint8_t _pendingSystemIdx;
    volatile uint8_t _pendingBandIdx;
    volatile uint8_t _pendingChannelIdx;
    volatile uint16_t _pendingFreqMhz;
    volatile bool _bandChannelDirty;
    
    // Cross-core threshold display update (written from loop core 1, read by UI task core 0)
    volatile uint8_t _pendingEnterRssi;
    volatile uint8_t _pendingExitRssi;
    volatile bool _thresholdDirty;
    
    // Cross-core buzzer request (written from UI task core 0, read by loop core 1)
    volatile uint16_t _pendingBuzzerMs;
    
    // Cross-core lap feed (written from loop() on core 1, read by UI task on core 0)
    static const uint8_t MAX_DISPLAY_LAPS = 5;
    volatile uint32_t _lapBuffer[5];   // Last 5 lap times in ms
    volatile uint8_t _lapCount;        // Total laps recorded
    volatile bool _lapsDirty;          // New lap data available
    
    // Update tracking
    uint32_t _lastGraphUpdate;

    // Internal helpers
    void createUI();
    void createRacingTab();
    void createPilotTab();
    void createCalibTab();
    void processRssiUpdate();
    void processLapUpdate();           // Called from UI task only
    void processBandChannelUpdate();   // Called from UI task only
    void processThresholdUpdate();     // Called from UI task only
    void updateScreenBrightness();
    
    // Countdown/finish overlay methods (called from UI task core 0 only)
    void startCountdown();
    void updateCountdown();
    void stopCountdown();
    void showFinish();
    void updateFinish();
    void stopFinish();

    // LVGL callbacks
    static void dispFlush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
    static void touchpadRead(lv_indev_drv_t* indev_driver, lv_indev_data_t* data);
    static void startBtnEvent(lv_event_t* e);
    static void stopBtnEvent(lv_event_t* e);
    static void clearBtnEvent(lv_event_t* e);
    static void systemPrevEvent(lv_event_t* e);
    static void systemNextEvent(lv_event_t* e);
    static void bandPrevEvent(lv_event_t* e);
    static void bandNextEvent(lv_event_t* e);
    static void channelPrevEvent(lv_event_t* e);
    static void channelNextEvent(lv_event_t* e);
    static void enterDecEvent(lv_event_t* e);
    static void enterIncEvent(lv_event_t* e);
    static void exitDecEvent(lv_event_t* e);
    static void exitIncEvent(lv_event_t* e);
    static void brightnessSliderEvent(lv_event_t* e);
};
