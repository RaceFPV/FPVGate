#ifndef LAPTIMER_H
#define LAPTIMER_H

#include "RX5808.h"
#include "buzzer.h"
#include "config.h"
#include "kalman.h"
#include "led.h"

// Forward declarations to avoid circular dependency
struct Track;
class WebhookManager;
class SweepManager;

// Constants
#define LAPTIMER_LAP_HISTORY 10
#define LAPTIMER_RSSI_HISTORY 100
#define LAPTIMER_CALIBRATION_HISTORY 5000  // Increased buffer for longer recordings

// Multi-pilot lap data structure
typedef struct {
    uint32_t lapTimes[LAPTIMER_LAP_HISTORY];
    uint32_t startTimeMs;
    uint32_t raceStartTimeMs;
    uint8_t lapCount;
    bool lapCountWraparound;
    uint8_t rssiPeak;
    uint32_t rssiPeakTimeMs;
    bool enteredGate;
    bool gateExited;
    uint8_t enterHoldSamples;
    uint32_t enterHoldStartMs;
    bool lapAvailable;
    uint32_t lastLapTime;  // Most recent lap time for this pilot
} pilot_lap_data_t;

typedef enum {
    STOPPED,
    WAITING,
    RUNNING,
    CALIBRATION_WIZARD
} laptimer_state_e;

class LapTimer {
   public:
    void init(Config *config, RX5808 *rx5808, Buzzer *buzzer, Led *l, WebhookManager *webhook = nullptr);
    void start();
    void stop();
    void handleLapTimerUpdate(uint32_t currentTimeMs);
    uint8_t getRssi();
    uint32_t getLapTime();
    bool isLapAvailable();
    
    // Calibration wizard methods
    void startCalibrationWizard();
    void stopCalibrationWizard();
    uint16_t getCalibrationRssiCount();
    uint8_t getCalibrationRssi(uint16_t index);
    uint32_t getCalibrationTimestamp(uint16_t index);
    
    // Track/distance methods
    void setTrack(Track* track);
    float getTotalDistance();
    float getDistanceRemaining();
    Track* getSelectedTrack();
    
    // Multi-pilot methods
    void setSweepManager(SweepManager* sweep);
    void setActivePilot(uint8_t pilot);  // 0 or 1
    uint8_t getActivePilot();
    uint32_t getLapTime(uint8_t pilotIndex);  // Get specific pilot's last lap
    uint8_t getLapCount(uint8_t pilotIndex);  // Get specific pilot's lap count
    uint32_t* getLapTimes(uint8_t pilotIndex);  // Get pilot's lap times array
    bool isLapAvailable(uint8_t pilotIndex);  // Check if pilot has new lap
    void clearLapAvailable(uint8_t pilotIndex);  // Clear lap available flag
    uint8_t getLastPilotWithLap();  // Get which pilot triggered the last lap (for events)

   private:
    laptimer_state_e state = STOPPED;
    RX5808 *rx;
    Config *conf;
    Buzzer *buz;
    Led *led;
    WebhookManager *webhooks;
    KalmanFilter filter;
    boolean lapCountWraparound;
    uint32_t raceStartTimeMs;
    uint32_t startTimeMs;
    uint8_t lapCount;
    uint8_t rssiCount;
    uint32_t lapTimes[LAPTIMER_LAP_HISTORY];
    uint8_t rssi[LAPTIMER_RSSI_HISTORY];
    uint8_t rssi_window[7];  // Medium window for moving average (5 is lower latency)
    uint8_t rssi_window_index;
    uint8_t lastLpRssi;

    uint8_t rssiPeak;
    uint32_t rssiPeakTimeMs;

    // Gate state tracking / debounce helpers
    bool gateExited;          // True when we're confidently outside the gate region
    bool enteredGate;         // True once we have crossed the enter threshold
    uint8_t enterHoldSamples; // Number of consecutive samples at/above enter
    uint32_t enterHoldStartMs;

    // Debug helpers (last processed RSSI chain)
    uint8_t lastRawRssi;
    uint8_t lastKalmanRssi;
    uint8_t lastAvgRssi;
    uint8_t prevAvgRssi;
    uint32_t lastRaceDebugPrintMs;

    bool lapAvailable = false;
    
    // Calibration wizard data
    uint16_t calibrationRssiCount;
    uint8_t calibrationRssi[LAPTIMER_CALIBRATION_HISTORY];
    uint32_t calibrationTimestamps[LAPTIMER_CALIBRATION_HISTORY];
    uint32_t lastCalibrationSampleMs;  // Track when last sample was taken
    
    // Track/distance tracking
    Track* selectedTrack;
    float totalDistanceTravelled;
    float distanceRemaining;
    
    // Multi-pilot support
    SweepManager* sweepMgr;
    uint8_t activePilot;              // 0 = pilot1, 1 = pilot2 (only used in multi-pilot mode)
    uint8_t lastLapPilot;             // Which pilot triggered the most recent lap
    pilot_lap_data_t pilotData[2];    // Per-pilot lap tracking data
    
    // Get RSSI thresholds for current pilot (multi-pilot aware)
    uint8_t getCurrentEnterRssi();
    uint8_t getCurrentExitRssi();

    void lapPeakCapture();
    bool lapPeakCaptured();
    void lapPeakReset();

    void startLap();
    void finishLap();
    
    // Multi-pilot helpers
    void initPilotData(uint8_t pilotIndex);
    void finishLapMultiPilot(uint8_t pilotIndex);
};

#endif
