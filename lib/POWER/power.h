#ifndef POWER_H
#define POWER_H

#include <Arduino.h>

// Optional feature: Physical power toggle switch for light sleep
// Enable with -DENABLE_POWER_SWITCH in build flags
// Requires a 3-pin SPDT slide switch wired to GPIO7
//
// Hardware wiring:
//   - Middle pin (COM) -> GPIO7
//   - One outer pin -> GND (OFF position)
//   - Other outer pin -> NC (ON position, uses internal pullup)
//
// Behavior:
//   - Switch ON (HIGH): Device runs normally
//   - Switch OFF (LOW): Device enters light sleep (~5-10mA)
//   - Wake: Flip switch back to ON (instant resume, no reboot)
//   - RAM, WiFi settings, and lap times are preserved across sleep

#ifdef ENABLE_POWER_SWITCH

class PowerManager {
public:
    PowerManager();
    
    // Initialize power management (call in setup() before other initialization)
    // Returns false if switch is OFF (device should immediately enter deep sleep)
    bool init(uint8_t switchPin, uint8_t backlightPin);
    
    // Check if power switch is in ON position
    bool isPowerOn();
    
    // Monitor switch and handle power state changes (call in loop())
    // Returns false if switch was flipped to OFF (device should gracefully shutdown)
    bool monitorSwitch();
    
    // Enter light sleep (turns off backlight and configures wake source)
    // This function DOES return when switch is flipped back to ON
    // Peripherals stay powered, RAM preserved, instant wake
    void enterDeepSleep();  // Name kept for compatibility, actually does light sleep
    
private:
    uint8_t _switchPin;
    uint8_t _backlightPin;
    uint32_t _lastCheckMs;
    bool _lastState;
    uint32_t _offDetectedMs;  // Timestamp when OFF was first detected
    uint32_t _wakeTime;       // Timestamp of wake from sleep
    static const uint32_t CHECK_INTERVAL_MS = 100;    // Check switch every 100ms
    static const uint32_t DEBOUNCE_MS = 200;          // Debounce time (200ms to avoid false triggers)
    static const uint32_t WAKE_COOLDOWN_MS = 3000;    // Don't check switch for 3s after wake
};

#endif // ENABLE_POWER_SWITCH

#endif // POWER_H
