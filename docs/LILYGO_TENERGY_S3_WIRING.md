# LilyGO T-Energy S3 Wiring Guide

## Overview

The LilyGO T-Energy S3 is an ESP32-S3 development board with built-in battery management and charging circuitry. It features a 100K/100K voltage divider on GPIO 4 for battery monitoring.

## Key Features

- **ESP32-S3** dual-core processor (240 MHz)
- **8MB Flash** / **8MB PSRAM**
- **Built-in LiPo battery connector** with charging IC
- **Battery voltage monitoring** on GPIO 4
- **USB-C** connector for programming and charging
- **18650 battery holder** (depending on variant)

## Pin Conflicts & Considerations

### Battery Voltage Monitoring
- **GPIO 4** is hardwired to battery voltage divider (100K/100K)
- Cannot be used for other purposes
- Voltage divider: 2:1 ratio (battery 3.0V-4.2V → ADC 1.5V-2.1V)

### Strapping Pins to Avoid
- **GPIO 0**: Boot mode selection (has pullup)
- **GPIO 3**: JTAG enable/disable (causes boot issues if unstable)
- **GPIO 45**: VDD_SPI voltage (do not use)
- **GPIO 46**: Boot mode (has pulldown)

### Safe GPIO Pins
- **GPIO 1, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15** - General purpose I/O
- **GPIO 48** - RGB LED (WS2812)

---

## FPVGate Recommended Pinout

### Component Connections

| Component | GPIO | Function | Notes |
|-----------|------|----------|-------|
| **RX5808 RSSI** | 4 | Analog input | SAME as DevKitC |
| **RX5808 CH1 (DATA)** | 10 | SPI data | Same as DevKitC |
| **RX5808 CH2 (SELECT)** | 11 | SPI chip select | Same as DevKitC |
| **RX5808 CH3 (CLOCK)** | 12 | SPI clock | Same as DevKitC |
| **Battery Voltage** | 3 | Analog input | DIFFERENT: Built-in divider (DO NOT CHANGE) |
| **Buzzer** | 5 | PWM output | Same as DevKitC |
| **SD Card CS** | 39 | SPI chip select | Same as DevKitC |
| **SD Card MOSI** | 35 | SPI data out | Same as DevKitC |
| **SD Card SCK** | 36 | SPI clock | Same as DevKitC |
| **SD Card MISO** | 37 | SPI data in | Same as DevKitC |
| **External LED** | 2 | Digital output | Same as DevKitC |
| **RGB LED** | 48 | WS2812 data | Same as DevKitC |
| **Mode Switch** | 9 | Digital input | Same as DevKitC |

### Power Connections

| Pin | Function | Notes |
|-----|----------|-------|
| **5V** | Power output | From USB or battery (via boost converter) |
| **3.3V** | Regulated output | For 3.3V components |
| **GND** | Ground | Common ground for all components |

---

## Wiring Diagram

```
LilyGO T-Energy S3    RX5808         SD Card Module
┌─────────────────┐
│                 │
│  GPIO4  ────────┼────── RSSI (SAME as DevKitC)
│  GPIO10 ────────┼────── CH1 (DATA)
│  GPIO11 ────────┼────── CH2 (SELECT)
│  GPIO12 ────────┼────── CH3 (CLOCK)
│  GPIO3  ────────┼────── (Battery - DO NOT CONNECT)
│  GPIO5  ────────┼────── Buzzer (+)
│  GPIO39 ────────┼──────────────────── CS
│  GPIO35 ────────┼──────────────────── MOSI
│  GPIO36 ────────┼──────────────────── SCK
│  GPIO37 ────────┼──────────────────── MISO
│  GPIO2  ────────┼────── LED (+)
│  GPIO48 ────────┼────── WS2812 Data
│                 │
│  5V     ────────┼────── +5V ──────── VCC
│  GND    ────────┼────── GND ──────── GND
│                 │
└─────────────────┘
```

---

## Component Details

### RX5808 Module (5.8GHz Receiver)

**Important:** RX5808 must be modified for SPI mode (bridge R16 resistor).

**Wiring:**
```
RX5808 Pin    →    T-Energy S3
─────────────────────────────
VCC (+5V)     →    5V
GND           →    GND
RSSI          →    GPIO 4 (SAME as DevKitC)
SPI_DATA      →    GPIO 10 (SAME as DevKitC)
SPI_SEL       →    GPIO 11 (SAME as DevKitC)
SPI_CLK       →    GPIO 12 (SAME as DevKitC)
```

### MicroSD Card Module

**Format:** FAT32  
**Capacity:** 1GB - 32GB recommended

**Wiring:**
```
SD Card Pin    →    T-Energy S3
─────────────────────────────
CS            →    GPIO 39 (SAME as DevKitC)
MOSI          →    GPIO 35 (SAME as DevKitC)
SCK           →    GPIO 36 (SAME as DevKitC)
MISO          →    GPIO 37 (SAME as DevKitC)
VCC           →    3.3V
GND           →    GND
```

### Buzzer (Optional)

**Type:** Passive buzzer (PWM-controlled)

**Wiring:**
```
Buzzer Pin    →    T-Energy S3
─────────────────────────────
+             →    GPIO 5 (SAME as DevKitC)
-             →    GND
```

**Note:** Add 100Ω resistor in series to reduce volume if needed.

### External LED (Optional)

**Type:** Standard LED (any color)

**Wiring:**
```
LED Pin       →    T-Energy S3
─────────────────────────────
Anode (+)     →    GPIO 2 (SAME as DevKitC, via 220Ω resistor)
Cathode (-)   →    GND
```

### RGB LED (Built-in)

The T-Energy S3 may have a built-in WS2812 RGB LED on GPIO 48. This is configured automatically in the firmware.

---

## Battery Voltage Monitoring

### Voltage Divider Circuit

The T-Energy S3 has a built-in 100K/100K voltage divider on **GPIO 4**:

```
Battery (+)
    │
   100K
    ├────── GPIO 4 (ADC)
   100K
    │
   GND
```

### Voltage Calculation

- **Battery Range:** 3.0V (empty) to 4.2V (full)
- **ADC Input Range:** 1.5V to 2.1V (after divider)
- **ADC Resolution:** 12-bit (0-4095)
- **Reference Voltage:** 3.3V

**Formula:**
```
Battery Voltage = (ADC_Raw / 4095) × 3.3V × 2
```

### Calibration

The default settings are:
- `VBAT_SCALE = 2` (for 2:1 divider)
- `VBAT_ADD = 0` (no offset)

If voltage readings are inaccurate:
1. Measure actual battery voltage with multimeter
2. Compare to reading in web interface
3. Adjust `VBAT_ADD` in `config.h` (lines 71-72)

**Example:** If multimeter reads 3.7V but device shows 3.5V, set `VBAT_ADD = 2` (adds 0.2V).

---

## Building & Flashing

### PlatformIO

```bash
# Build for T-Energy S3
pio run -e LilyGOTEnergyS3 -t upload

# Upload filesystem
pio run -e LilyGOTEnergyS3 -t uploadfs
```

### Manual Flash (esptool)

```bash
# Download prebuilt binaries from releases page
esptool.py --chip esp32s3 --port COM3 write_flash -z \
  0x0 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin \
  0x410000 littlefs.bin
```

---

## Troubleshooting

### Battery Voltage Not Reading

**Symptoms:** Always shows 0V or incorrect value

**Solutions:**
1. GPIO 4 is hardwired - cannot be reassigned
2. Check that battery is connected
3. Verify battery voltage with multimeter (should be 3.0V-4.2V)
4. Check `VBAT_SCALE` and `VBAT_ADD` values in `config.h`
5. Ensure battery monitoring is enabled in `main.cpp` (not commented out)

### GPIO 3 Boot Issues

**Symptoms:** Device won't boot or boots inconsistently

**Solution:** Do NOT use GPIO 3 for any connections. It's a strapping pin.

### RX5808 Not Working

**Symptoms:** RSSI always 0 or not tuning

**Solutions:**
1. Verify SPI mod (R16 bridge) on RX5808
2. Check wiring: GPIO 5, 6, 7 for SPI communication
3. Ensure 5V power to RX5808
4. Run self-test in web interface

### SD Card Not Detected

**Symptoms:** "SD card failed" error

**Solutions:**
1. Format card as FAT32
2. Check wiring: GPIO 10 (CS), 11 (MOSI), 12 (SCK), 13 (MISO)
3. Use 3.3V power, not 5V
4. Try different SD card (use 32GB or smaller)

---

## Additional Resources

- [T-Energy S3 Datasheet](https://github.com/Xinyuan-LilyGO/T-Energy)
- [ESP32-S3 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
- [FPVGate Main Documentation](../README.md)
- [Hardware Guide](HARDWARE_GUIDE.md)
- [Getting Started](GETTING_STARTED.md)
