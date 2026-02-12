#include "sweep.h"
#include "debug.h"

void SweepManager::init(Config* config, RX5808* rx5808) {
    conf = config;
    rx = rx5808;
    
    state = SWEEP_IDLE;
    activePilot = 0;
    pilotSwitched = false;
    manualMode = false;
    lastSwitchTimeMs = 0;
    tuneStartTimeMs = 0;
    
    DEBUG("SweepManager initialized\n");
}

void SweepManager::handleSweep(uint32_t currentTimeMs) {
    // Early exit if multi-pilot mode is disabled - zero overhead
    if (!conf->getMultiPilotEnabled()) {
        if (state != SWEEP_IDLE) {
            state = SWEEP_IDLE;
            DEBUG("SweepManager: Multi-pilot disabled, returning to idle\n");
        }
        return;
    }
    
    // Don't sweep in manual/calibration mode
    if (manualMode) {
        return;
    }
    
    uint16_t dwellMs = conf->getSweepDwellMs();
    
    switch (state) {
        case SWEEP_IDLE:
            // Just enabled multi-pilot mode - start with pilot 1
            DEBUG("SweepManager: Starting sweep mode, activating Pilot 1\n");
            activePilot = 0;
            state = SWEEP_TUNING;
            tuneStartTimeMs = currentTimeMs;
            rx->setFrequency(getPilotFrequency(0));
            break;
            
        case SWEEP_TUNING:
            // Wait for tuning to complete (RX5808_MIN_TUNETIME)
            if ((currentTimeMs - tuneStartTimeMs) >= RX5808_MIN_TUNETIME) {
                // Tuning complete, now dwelling on this pilot
                state = (activePilot == 0) ? SWEEP_PILOT1 : SWEEP_PILOT2;
                lastSwitchTimeMs = currentTimeMs;
                pilotSwitched = true;
                DEBUG("SweepManager: Tuned to Pilot %d, freq=%u\n", 
                      activePilot + 1, getPilotFrequency(activePilot));
            }
            break;
            
        case SWEEP_PILOT1:
        case SWEEP_PILOT2:
            // Check if dwell time has elapsed
            if ((currentTimeMs - lastSwitchTimeMs) >= dwellMs) {
                switchPilot(currentTimeMs);
            }
            break;
    }
}

void SweepManager::switchPilot(uint32_t currentTimeMs) {
    // Toggle to other pilot
    activePilot = (activePilot == 0) ? 1 : 0;
    
    // Start frequency change
    uint16_t newFreq = getPilotFrequency(activePilot);
    rx->setFrequency(newFreq);
    
    state = SWEEP_TUNING;
    tuneStartTimeMs = currentTimeMs;
    
    DEBUG("SweepManager: Switching to Pilot %d, freq=%u\n", activePilot + 1, newFreq);
}

uint8_t SweepManager::getActivePilot() {
    return activePilot;
}

bool SweepManager::didPilotSwitch() {
    if (pilotSwitched) {
        pilotSwitched = false;
        return true;
    }
    return false;
}

bool SweepManager::isRssiValid() {
    // RSSI is only valid when we're dwelling on a pilot, not during tuning
    return (state == SWEEP_PILOT1 || state == SWEEP_PILOT2);
}

void SweepManager::activatePilot(uint8_t pilotIndex) {
    if (pilotIndex > 1) pilotIndex = 0;
    
    manualMode = true;
    activePilot = pilotIndex;
    
    // Set frequency immediately
    uint16_t freq = getPilotFrequency(pilotIndex);
    rx->setFrequency(freq);
    
    // In manual mode, go directly to the pilot state after a brief settling time
    // We don't use SWEEP_TUNING because handleSweep returns early in manual mode
    state = (pilotIndex == 0) ? SWEEP_PILOT1 : SWEEP_PILOT2;
    lastSwitchTimeMs = millis();
    pilotSwitched = true;
    
    DEBUG("SweepManager: Manual activation of Pilot %d, freq=%u\n", pilotIndex + 1, freq);
}

void SweepManager::resume() {
    manualMode = false;
    lastSwitchTimeMs = millis();
    state = (activePilot == 0) ? SWEEP_PILOT1 : SWEEP_PILOT2;
    
    DEBUG("SweepManager: Resuming automatic sweep\n");
}

bool SweepManager::isManualMode() {
    return manualMode;
}

sweep_state_e SweepManager::getState() {
    return state;
}

void SweepManager::reset() {
    // Reset to pilot 1 at race start
    activePilot = 0;
    pilotSwitched = false;
    lastSwitchTimeMs = millis();
    tuneStartTimeMs = 0;
    
    if (conf->getMultiPilotEnabled() && !manualMode) {
        state = SWEEP_TUNING;
        tuneStartTimeMs = millis();
        rx->setFrequency(getPilotFrequency(0));
        DEBUG("SweepManager: Reset - starting with Pilot 1\n");
    }
}

uint16_t SweepManager::getPilotFrequency(uint8_t pilotIndex) {
    if (pilotIndex == 0) {
        return conf->getPilot1()->frequency;
    } else {
        return conf->getPilot2()->frequency;
    }
}
