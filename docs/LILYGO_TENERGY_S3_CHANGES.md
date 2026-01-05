# LilyGO T-Energy S3 Pinout Changes

## Overview
The LilyGO T-Energy S3 pinout has been **updated to match the ESP32-S3 DevKitC** as closely as possible. This makes it easier for users who have already wired their boards according to the DevKitC pinout.

---

## Changes Summary

### SAME AS ESP32-S3 DevKitC (No changes needed)

| Component | GPIO | Notes |
|-----------|------|-------|
| **LED** | 2 | Changed from GPIO 15 → GPIO 2 |
| **RGB LED** | 48 | No change |
| **Buzzer** | 5 | Changed from GPIO 8 → GPIO 5 |
| **RX5808 CH1 (DATA)** | 10 | Changed from GPIO 5 → GPIO 10 |
| **RX5808 CH2 (SELECT)** | 11 | Changed from GPIO 6 → GPIO 11 |
| **RX5808 CH3 (CLOCK)** | 12 | Changed from GPIO 7 → GPIO 12 |
| **SD Card CS** | 39 | Changed from GPIO 10 → GPIO 39 |
| **SD Card SCK** | 36 | Changed from GPIO 12 → GPIO 36 |
| **SD Card MOSI** | 35 | Changed from GPIO 11 → GPIO 35 |
| **SD Card MISO** | 37 | Changed from GPIO 13 → GPIO 37 |
| **Mode Switch** | 9 | No change |

### DIFFERENT FROM ESP32-S3 DevKitC (Hardware constraint)

| Component | T-Energy S3 | DevKitC | Reason |
|-----------|-------------|---------|--------|
| **Battery Voltage** | GPIO 4 | GPIO 1 | T-Energy has built-in voltage divider on GPIO 4 |
| **RX5808 RSSI** | GPIO 1 | GPIO 4 | Moved to GPIO 1 because GPIO 4 is used by battery |

---

## What Changed?

### OLD Pinout (Original T-Energy S3)
```
LED:           GPIO 15
Buzzer:        GPIO 8
RX5808 RSSI:   GPIO 1
RX5808 DATA:   GPIO 5
RX5808 SELECT: GPIO 6
RX5808 CLOCK:  GPIO 7
Battery:       GPIO 4 (hardwired)
SD CS:         GPIO 10
SD SCK:        GPIO 12
SD MOSI:       GPIO 11
SD MISO:       GPIO 13
```

### NEW Pinout (Matches DevKitC except RSSI & Battery)
```
LED:           GPIO 2  SAME as DevKitC
Buzzer:        GPIO 5  SAME as DevKitC
RX5808 RSSI:   GPIO 1  DIFFERENT: GPIO 4 used by battery
RX5808 DATA:   GPIO 10 SAME as DevKitC
RX5808 SELECT: GPIO 11 SAME as DevKitC
RX5808 CLOCK:  GPIO 12 SAME as DevKitC
Battery:       GPIO 4  DIFFERENT: T-Energy hardwired
SD CS:         GPIO 39 SAME as DevKitC
SD SCK:        GPIO 36 SAME as DevKitC
SD MOSI:       GPIO 35 SAME as DevKitC
SD MISO:       GPIO 37 SAME as DevKitC
```

---

## For Users Who Already Wired to DevKitC Pinout

### What Stays the Same
If you wired your T-Energy S3 according to the DevKitC pinout, **you only need to rewire 1 pin**:

- **RX5808 RSSI**: Move from GPIO 4 → GPIO 1

### Why This Change?
GPIO 4 on the T-Energy S3 is **hardwired** to the battery voltage divider circuit on the board. You cannot use it for anything else. Therefore, RSSI had to be moved to GPIO 1.

### Battery Monitoring
- **GPIO 4** is automatically connected to the battery (built-in on the board)
- No external wiring needed for battery voltage sensing
- The firmware will automatically read battery voltage and display it in the web interface

---

## Complete Wiring Guide

### ESP32-S3 DevKitC → T-Energy S3 Migration

| DevKitC Wiring | T-Energy S3 Wiring | Change? |
|----------------|-------------------|---------|
| RSSI → GPIO 4 | RSSI → GPIO 1 | MOVE TO GPIO 1 |
| CH1 → GPIO 10 | CH1 → GPIO 10 | Same |
| CH2 → GPIO 11 | CH2 → GPIO 11 | Same |
| CH3 → GPIO 12 | CH3 → GPIO 12 | Same |
| Buzzer → GPIO 5 | Buzzer → GPIO 5 | Same |
| LED → GPIO 2 | LED → GPIO 2 | Same |
| SD CS → GPIO 39 | SD CS → GPIO 39 | Same |
| SD SCK → GPIO 36 | SD SCK → GPIO 36 | Same |
| SD MOSI → GPIO 35 | SD MOSI → GPIO 35 | Same |
| SD MISO → GPIO 37 | SD MISO → GPIO 37 | Same |
| Battery → GPIO 1 (external) | Battery → GPIO 4 (built-in) | No wire needed |

---

## Benefits

1. **Minimal rewiring**: Only 1 wire needs to be moved (RSSI from GPIO 4 → GPIO 1)
2. **Built-in battery monitoring**: GPIO 4 automatically reads battery voltage
3. **Standard pinout**: Matches DevKitC for easier documentation and troubleshooting
4. **Hardware SPI**: Uses same SPI pins as DevKitC for SD card

---

## See Also

- [Complete T-Energy S3 Wiring Guide](LILYGO_TENERGY_S3_WIRING.md)
- [ESP32-S3 DevKitC Wiring](HARDWARE_GUIDE.md)
- [Getting Started Guide](GETTING_STARTED.md)
