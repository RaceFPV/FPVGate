#ifndef ROTORHAZARD_H
#define ROTORHAZARD_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Maximum number of queued lap events before dropping
#define RH_QUEUE_SIZE 8
// Standard RotorHazard HTTP port
#define RH_DEFAULT_PORT 5000
// HTTP timeout for lap POST and time-sync GET requests (ms)
#define RH_HTTP_TIMEOUT_MS 1500
// How often to re-sync the clock offset with RH (ms)
#define RH_SYNC_INTERVAL_MS 30000

class Config;  // forward declaration

// A queued gate crossing: raw millis() values captured at crossing time.
struct PendingLap {
    uint32_t crossingMillis;   // millis() at the moment of the gate crossing
    uint32_t raceStartMillis;  // millis() when the current race was started
};

class RHManager {
   public:
    RHManager();

    // Call once in setup() after config is loaded
    void init(Config* config);

    // Call in main loop() - performs clock sync and drains pending lap queue
    void process();

    // Queue a lap/crossing event.
    //   crossingMillis  - raw millis() at the moment of crossing
    //   raceStartMillis - raw millis() when the current race started
    void triggerLap(uint32_t crossingMillis, uint32_t raceStartMillis);

    // Enable / disable without changing config permanently
    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Returns true if the last lap POST succeeded
    bool isConnected() const;

    // Returns true if at least one successful clock sync has been performed
    bool isClockSynced() const;

    // Returns the computed clock offset (millis() + offset = RH monotonic ms)
    int64_t getClockOffsetMs() const;

    // Returns the round-trip time of the most recent successful sync (ms)
    uint32_t getLastSyncRttMs() const;

    // Request an immediate clock sync on the next process() tick
    void requestSync();

    // Call when WiFi newly connects - triggers an immediate clock sync
    void onWifiConnected();

    // Notify RotorHazard to stage (arm) a new race.
    // Fire-and-forget: the request is queued and sent on the next process() tick.
    void startRace();

    // Notify RotorHazard to stop the current race.
    // Fire-and-forget: the request is queued and sent on the next process() tick.
    void stopRace();

   private:
    Config* conf;

    bool enabled;
    bool lastPostOk;
    uint8_t pendingCount;
    PendingLap pendingLaps[RH_QUEUE_SIZE];

    // Clock synchronisation state
    int64_t  clockOffsetMs;   // add to millis() to get RH monotonic ms
    bool     clockSynced;     // true once first sync succeeds
    bool     needsSync;       // set true on WiFi connect for immediate sync
    uint32_t lastSyncMs;      // millis() when last sync was performed
    uint32_t lastSyncRttMs;   // round-trip time of the most recent successful sync (ms)

    void sendPendingLaps();
    void syncClock();
    void postRaceControl(const char* path);

    bool pendingRaceStart;
    bool pendingRaceStop;
};

#endif // ROTORHAZARD_H
