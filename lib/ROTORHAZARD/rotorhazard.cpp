#include "rotorhazard.h"
#include "config.h"
#include "debug.h"

RHManager::RHManager()
    : conf(nullptr),
      enabled(false),
      lastPostOk(false),
      pendingCount(0),
      clockOffsetMs(0),
      clockSynced(false),
      needsSync(false),
      lastSyncMs(0) {
    memset(pendingLaps, 0, sizeof(pendingLaps));
}

void RHManager::init(Config* config) {
    conf = config;
    enabled = (conf->getRhEnabled() == 1);
    DEBUG("[RH] Manager initialized, enabled=%d, host=%s, node=%d\n",
          enabled, conf->getRhHostIP(), conf->getRhNodeIndex());
}

void RHManager::setEnabled(bool en) {
    enabled = en;
    if (!enabled) {
        lastPostOk = false;
        DEBUG("[RH] Disabled\n");
    }
}

bool RHManager::isEnabled()     const { return enabled; }
bool RHManager::isConnected()   const { return lastPostOk; }
bool RHManager::isClockSynced() const { return clockSynced; }

void RHManager::onWifiConnected() {
    lastPostOk = false;
    needsSync  = true;   // trigger an immediate clock sync on next process() tick
    DEBUG("[RH] WiFi connected - clock sync queued\n");
}

void RHManager::triggerLap(uint32_t crossingMillis, uint32_t raceStartMillis) {
    if (!enabled) return;
    if (pendingCount < RH_QUEUE_SIZE) {
        pendingLaps[pendingCount].crossingMillis  = crossingMillis;
        pendingLaps[pendingCount].raceStartMillis = raceStartMillis;
        pendingCount++;
        DEBUG("[RH] Lap queued crossingMs=%u raceStartMs=%u (pending=%d)\n",
              crossingMillis, raceStartMillis, pendingCount);
    } else {
        DEBUG("[RH] Lap queue full, dropping event\n");
    }
}

void RHManager::process() {
    if (!enabled) return;
    if (WiFi.status() != WL_CONNECTED) return;

    // Sync clock when requested (WiFi just connected) or when interval has elapsed,
    // but only while no laps are waiting so we don't delay a crossing report.
    bool intervalDue = (millis() - lastSyncMs) >= RH_SYNC_INTERVAL_MS;
    if ((needsSync || intervalDue) && pendingCount == 0) {
        syncClock();
    }

    if (pendingCount > 0) {
        sendPendingLaps();
    }
}

// ---------------------------------------------------------------------------
// NTP-style clock synchronisation
// ---------------------------------------------------------------------------
// GETs /fpvgate/time from the RH plugin and computes an offset such that:
//   millis() + clockOffsetMs  ≈  RH monotonic milliseconds
// ---------------------------------------------------------------------------
void RHManager::syncClock() {
    const char* host = conf->getRhHostIP();
    if (host[0] == '\0') return;

    char url[72];
    snprintf(url, sizeof(url), "http://%s:%d/fpvgate/time", host, RH_DEFAULT_PORT);

    uint32_t t1 = millis();

    HTTPClient http;
    http.setConnectTimeout(RH_HTTP_TIMEOUT_MS);
    http.setTimeout(RH_HTTP_TIMEOUT_MS);

    if (!http.begin(url)) {
        http.end();
        DEBUG("[RH] syncClock: http.begin() failed\n");
        needsSync  = false;
        lastSyncMs = millis();
        return;
    }

    int code = http.GET();
    uint32_t t2 = millis();

    if (code != 200) {
        http.end();
        DEBUG("[RH] syncClock: HTTP %d\n", code);
        needsSync  = false;
        lastSyncMs = t2;
        return;
    }

    String payload = http.getString();
    http.end();

    // Parse {"monotonic_ms": <value>} - find the number after the colon
    int idx = payload.indexOf("monotonic_ms");
    if (idx < 0) {
        DEBUG("[RH] syncClock: key not found in response\n");
        needsSync  = false;
        lastSyncMs = t2;
        return;
    }
    idx = payload.indexOf(':', idx);
    if (idx < 0) {
        needsSync  = false;
        lastSyncMs = t2;
        return;
    }
    // Skip whitespace after the colon
    idx++;
    while (idx < (int)payload.length() &&
           (payload[idx] == ' ' || payload[idx] == '\t')) {
        idx++;
    }
    int64_t serverMs = (int64_t)atoll(payload.c_str() + idx);

    uint32_t rtt      = t2 - t1;
    int64_t  midpoint = (int64_t)t1 + (int64_t)(rtt / 2);
    clockOffsetMs = serverMs - midpoint;
    clockSynced   = true;
    needsSync     = false;
    lastSyncMs    = t2;

    DEBUG("[RH] Clock synced: offset=%lld ms, rtt=%u ms\n",
          (long long)clockOffsetMs, rtt);
}

// ---------------------------------------------------------------------------
// Send all queued lap crossings to RotorHazard
// ---------------------------------------------------------------------------
void RHManager::sendPendingLaps() {
    const char* host = conf->getRhHostIP();
    if (host[0] == '\0') {
        DEBUG("[RH] No host configured, dropping %d pending lap(s)\n", pendingCount);
        pendingCount = 0;
        return;
    }

    uint8_t nodeIndex = conf->getRhNodeIndex();

    char url[72];
    snprintf(url, sizeof(url), "http://%s:%d/fpvgate/lap", host, RH_DEFAULT_PORT);

    while (pendingCount > 0) {
        // Consume oldest entry first (FIFO)
        PendingLap entry = pendingLaps[0];
        for (uint8_t i = 1; i < pendingCount; i++) {
            pendingLaps[i - 1] = pendingLaps[i];
        }
        pendingCount--;

        // Always compute raceTimeMs for the fallback path
        uint32_t raceTimeMs = entry.crossingMillis - entry.raceStartMillis;

        // Build JSON body - include timestampMs (absolute RH monotonic ms) when synced
        char body[80];
        if (clockSynced) {
            int64_t timestampMs = (int64_t)entry.crossingMillis + clockOffsetMs;
            snprintf(body, sizeof(body),
                     "{\"node\":%d,\"raceTimeMs\":%lu,\"timestampMs\":%lld}",
                     nodeIndex,
                     (unsigned long)raceTimeMs,
                     (long long)timestampMs);
        } else {
            snprintf(body, sizeof(body),
                     "{\"node\":%d,\"raceTimeMs\":%lu}",
                     nodeIndex,
                     (unsigned long)raceTimeMs);
        }

        HTTPClient http;
        http.setConnectTimeout(RH_HTTP_TIMEOUT_MS);
        http.setTimeout(RH_HTTP_TIMEOUT_MS);

        if (http.begin(url)) {
            http.addHeader("Content-Type", "application/json");
            int code = http.POST(body);
            http.end();

            if (code == 200) {
                lastPostOk = true;
                DEBUG("[RH] Lap posted OK (node=%d, raceTimeMs=%u, synced=%d)\n",
                      nodeIndex, raceTimeMs, (int)clockSynced);
            } else {
                lastPostOk = false;
                DEBUG("[RH] Lap POST failed (HTTP %d, node=%d)\n", code, nodeIndex);
            }
        } else {
            lastPostOk = false;
            DEBUG("[RH] http.begin() failed for %s\n", url);
        }
    }
}
