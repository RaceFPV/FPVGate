# LilyGO T-Energy S3 Pinout Verification

## Board Configuration

**Build Target:** `LilyGOTEnergyS3`  
**Base Board:** ESP32-S3  
**Special Feature:** Built-in battery management on GPIO 4

---

## Complete Pin Mapping

### RX5808 (5.8GHz Receiver)

| Signal | GPIO | ESP32-S3 DevKitC | Match? | Notes |
|--------|------|------------------|--------|-------|
| RSSI | 1 | GPIO 4 | NO | DIFFERENT: Moved to GPIO 1 (GPIO 4 used by battery) |
| CH1 (DATA) | 10 | GPIO 10 | YES | SAME as DevKitC |
| CH2 (SELECT) | 11 | GPIO 11 | YES | SAME as DevKitC |
| CH3 (CLOCK) | 12 | GPIO 12 | YES | SAME as DevKitC |

### SD Card (MicroSD Module)

| Signal | GPIO | ESP32-S3 DevKitC | Match? | Notes |
|--------|------|------------------|--------|-------|
| CS | 39 | GPIO 39 | YES | SAME as DevKitC |
| MOSI | 35 | GPIO 35 | YES | SAME as DevKitC |
| SCK | 36 | GPIO 36 | YES | SAME as DevKitC |
| MISO | 37 | GPIO 37 | YES | SAME as DevKitC |

### Peripherals

| Component | GPIO | ESP32-S3 DevKitC | Match? | Notes |
|-----------|------|------------------|--------|-------|
| LED | 2 | GPIO 2 | YES | SAME as DevKitC |
| RGB LED (WS2812) | 48 | GPIO 48 | YES | SAME as DevKitC |
| Buzzer | 5 | GPIO 5 | YES | SAME as DevKitC |
| Mode Switch | 9 | GPIO 9 | YES | SAME as DevKitC |
| Battery Voltage | 4 | GPIO 1 | NO | DIFFERENT: T-Energy has built-in divider on GPIO 4 |

---

## Configuration Values

### Battery Monitoring
```c
#define PIN_VBAT 4              // T-Energy built-in voltage divider
#define VBAT_SCALE 2            // 2:1 voltage divider (100K/100K)
#define VBAT_ADD 0              // Calibration offset (adjust if needed)
```

### Key Differences from DevKitC
1. **RX5808 RSSI**: GPIO 1 (instead of GPIO 4)
2. **Battery Voltage**: GPIO 4 (instead of GPIO 1)

**Reason:** GPIO 4 on T-Energy S3 is hardwired to battery voltage divider circuit

---

## Pin Availability Check

### Used Pins (T-Energy S3)
- GPIO 1: RX5808 RSSI
- GPIO 2: LED
- GPIO 4: Battery (hardwired on board)
- GPIO 5: Buzzer
- GPIO 9: Mode Switch
- GPIO 10: RX5808 CH1
- GPIO 11: RX5808 CH2
- GPIO 12: RX5808 CH3
- GPIO 35: SD MOSI
- GPIO 36: SD SCK
- GPIO 37: SD MISO
- GPIO 39: SD CS
- GPIO 48: RGB LED

### Safe Unused Pins
- GPIO 6, 7, 8, 13, 14, 15, 16, 17, 18, 21

### Avoid These Pins
- GPIO 0: Boot mode (strapping pin)
- GPIO 3: JTAG enable (strapping pin)
- GPIO 45: VDD_SPI voltage
- GPIO 46: Boot mode (strapping pin)

---

## Hardware Compatibility

### Compatible with ESP32-S3 DevKitC Wiring?
**YES - 90% compatible**

Only 1 wire needs to be moved:
- **RX5808 RSSI**: From GPIO 4 → GPIO 1

All other connections remain the same.

### Battery Monitoring
- **Automatic**: GPIO 4 is connected to battery on the T-Energy board
- **No external wiring needed**: Built-in voltage divider
- **Voltage range**: 3.0V - 4.2V (LiPo battery)
- **Web interface**: Displays real-time voltage in System Settings

---

## Build Instructions

### PlatformIO
```bash
# Build and flash
pio run -e LilyGOTEnergyS3 -t upload

# Upload filesystem
pio run -e LilyGOTEnergyS3 -t uploadfs
```

### Build Flags
```ini
-DLILYGO_TENERGY_S3=1
```

---

## Verification Checklist

### Before Flashing
- [ ] RX5808 RSSI connected to GPIO 1 (NOT GPIO 4)
- [ ] RX5808 CH1/CH2/CH3 connected to GPIO 10/11/12
- [ ] SD Card connected to GPIO 39/35/36/37
- [ ] LED connected to GPIO 2
- [ ] Buzzer connected to GPIO 5
- [ ] Battery plugged into T-Energy battery connector
- [ ] GPIO 4 left unconnected (hardwired to battery)

### After Flashing
- [ ] Device boots successfully
- [ ] Web interface accessible at `192.168.4.1` or `FPVGate_XXXX` network
- [ ] Battery voltage displays in System Settings
- [ ] RX5808 responds to frequency changes
- [ ] SD card detected (if installed)
- [ ] RGB LED functions (if installed)

---

## Troubleshooting

### Battery Voltage Shows 0V or Incorrect
1. Check battery is connected to T-Energy battery connector
2. Verify battery voltage with multimeter (should be 3.0V-4.2V)
3. Adjust `VBAT_ADD` in `config.h` if consistently off
4. Ensure GPIO 4 is NOT connected to anything external

### RSSI Always 0
1. Verify RX5808 RSSI wire is on GPIO 1 (NOT GPIO 4)
2. Check RX5808 has 5V power
3. Verify RX5808 SPI mod (R16 bridge) is done
4. Run self-test in web interface

### SD Card Not Detected
1. Verify SD card wiring: CS=39, MOSI=35, SCK=36, MISO=37
2. Format card as FAT32
3. Use 3.3V power to SD card (NOT 5V)
4. Try different SD card (use 32GB or smaller)

---

## Summary

The LilyGO T-Energy S3 configuration is **correct** and ready for use. The pinout matches the ESP32-S3 DevKitC standard except for:

1. **RX5808 RSSI on GPIO 1** (instead of GPIO 4)
2. **Battery voltage on GPIO 4** (T-Energy built-in)

Users who wired to DevKitC pinout only need to move one wire (RSSI) to use the T-Energy S3.
