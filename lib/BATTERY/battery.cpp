#include "battery.h"

#include <Arduino.h>

#include "debug.h"

void BatteryMonitor::init(uint8_t pin, float batScale, uint8_t batAdd, Buzzer *buzzer, Led *l) {
    buz = buzzer;
    led = l;
    vbatPin = pin;
    scale = batScale;
    add = batAdd;
    state = ALARM_OFF;
    memset(measurements, 0, sizeof(measurements));
    measurementIndex = 0;
    lastCheckTimeMs = millis();
    enableDebug = false;  // Start with debug disabled
    pinMode(vbatPin, INPUT);

    for (int i = 0; i < AVERAGING_SIZE; i++) {
        getBatteryVoltage();  // kick averaging sum up to speed.
    }
}

static uint16_t averageSum = 0;

uint16_t BatteryMonitor::getBatteryVoltage() {
    // 0-3.3V maps to 0-4095, battery voltage ranges from 4.2V to 3.0V, but the voltage is divided, so 2.1V - 1.5V
    volatile uint16_t raw = analogRead(vbatPin);
    averageSum = averageSum - measurements[measurementIndex];  // substract oldest val
    measurements[measurementIndex] = raw;                      // replace old with new val
    averageSum += raw;                                         // update averageSum
    measurementIndex = (measurementIndex + 1) % AVERAGING_SIZE;
    uint16_t averaged = round(averageSum / AVERAGING_SIZE);
    // Map ADC reading (0-4095) to voltage in tenths of volt (0-330 for 3.3V ref, multiplied by divider ratio)
    uint16_t scaled = map(averaged, 0, 4095, 0, (uint16_t)(33 * scale)) + add;
    if (enableDebug) {
        DEBUG("Battery raw:%u, avg:%u, scaled:%u (%.1fV)\n", raw, averaged, scaled, scaled / 10.0f);
    }
    return scaled;
}

void BatteryMonitor::checkBatteryState(uint32_t currentTimeMs, uint8_t alarmThreshold) {
    // Update debug flag based on whether monitoring is enabled
    // This persists across all getBatteryVoltage() calls
    enableDebug = (alarmThreshold > 0);
    
    switch (state) {
        case ALARM_OFF:
            if ((alarmThreshold > 0) && ((currentTimeMs - lastCheckTimeMs) > MONITOR_CHECK_TIME_MS)) {
                lastCheckTimeMs = currentTimeMs;
                if (getBatteryVoltage() <= alarmThreshold) {
                    state = ALARM_BEEPING;
                    buz->beep(MONITOR_BEEP_TIME_MS);
                    led->blink(MONITOR_BEEP_TIME_MS);
                }
            }
            break;
        case ALARM_BEEPING:
            if ((currentTimeMs - lastCheckTimeMs) > MONITOR_BEEP_TIME_MS) {
                lastCheckTimeMs = currentTimeMs;
                state = ALARM_IDLE;
            }
            break;
        case ALARM_IDLE:
            if ((currentTimeMs - lastCheckTimeMs) > MONITOR_BEEP_TIME_MS) {
                lastCheckTimeMs = currentTimeMs;
                if (getBatteryVoltage() <= alarmThreshold + 1) {  // add 0.1V of histeresis
                    state = ALARM_BEEPING;
                    buz->beep(MONITOR_BEEP_TIME_MS);
                } else {
                    led->off();
                    state = ALARM_OFF;
                }
            }
            break;
        default:
            break;
    }
}
