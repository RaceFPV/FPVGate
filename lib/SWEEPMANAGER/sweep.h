#ifndef SWEEP_H
#define SWEEP_H

#include <stdint.h>
#include "config.h"
#include "RX5808.h"

// Sweep state enumeration
typedef enum {
    SWEEP_IDLE,      // Not sweeping (single pilot mode or stopped)
    SWEEP_PILOT1,    // Currently tuned to pilot 1's frequency
    SWEEP_PILOT2,    // Currently tuned to pilot 2's frequency
    SWEEP_TUNING     // In the process of switching frequencies
} sweep_state_e;

/**
 * SweepManager handles frequency alternation between two pilots
 * when multi-pilot sweep mode is enabled.
 * 
 * This is a standalone mode that only activates when multiPilotEnabled=1.
 * When disabled, it has zero impact on normal single-pilot operation.
 */
class SweepManager {
public:
    /**
     * Initialize the sweep manager with config and RX5808 references
     */
    void init(Config* config, RX5808* rx5808);
    
    /**
     * Handle sweep timing - call this from the main loop
     * Only performs work when multiPilotEnabled is true
     * @param currentTimeMs Current time in milliseconds
     */
    void handleSweep(uint32_t currentTimeMs);
    
    /**
     * Get the currently active pilot (0 or 1)
     * @return 0 for pilot 1, 1 for pilot 2
     */
    uint8_t getActivePilot();
    
    /**
     * Check if we just switched pilots (for lap timer notification)
     * Calling this clears the flag.
     * @return true if pilot switched since last call
     */
    bool didPilotSwitch();
    
    /**
     * Check if RSSI readings are currently valid
     * (False during frequency tuning period)
     * @return true if RSSI is stable and valid
     */
    bool isRssiValid();
    
    /**
     * Force switch to a specific pilot (for calibration mode)
     * Disables automatic sweeping until resume() is called
     * @param pilotIndex 0 for pilot 1, 1 for pilot 2
     */
    void activatePilot(uint8_t pilotIndex);
    
    /**
     * Resume automatic sweeping after manual pilot activation
     */
    void resume();
    
    /**
     * Check if sweep manager is in manual mode (not sweeping)
     * @return true if in manual/calibration mode
     */
    bool isManualMode();
    
    /**
     * Get the current sweep state
     * @return Current sweep state
     */
    sweep_state_e getState();
    
    /**
     * Reset sweep timing (e.g., when race starts)
     */
    void reset();

private:
    Config* conf;
    RX5808* rx;
    
    sweep_state_e state;
    uint8_t activePilot;           // 0 = pilot1, 1 = pilot2
    bool pilotSwitched;            // Flag set when pilot changes
    bool manualMode;               // True when in calibration/manual mode
    
    uint32_t lastSwitchTimeMs;     // When we last switched pilots
    uint32_t tuneStartTimeMs;      // When frequency change started
    
    // Switch to the other pilot
    void switchPilot(uint32_t currentTimeMs);
    
    // Get frequency for a pilot
    uint16_t getPilotFrequency(uint8_t pilotIndex);
};

#endif // SWEEP_H
