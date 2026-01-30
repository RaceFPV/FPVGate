# FPVGate v1.5.6 - Release Notes

**Release Date:** February 1, 2026

## Battery Monitoring Improvements

### Enhanced Voltage Precision
- **Upgraded battery voltage data type from `uint8_t` to `uint16_t`**
  - Increased precision from 0-255 range to 0-65535 range
  - Enables accurate voltage readings to tenths of a volt (e.g., 4.2V)
  - Better resolution for battery state monitoring and low voltage detection
  - Improved debug output with actual voltage display (e.g., "4.2V" instead of raw values)

### Configurable Voltage Divider
- **Changed voltage divider scale parameter from `uint8_t` to `float`**
  - Allows precise voltage divider ratio configuration (e.g., 2.0, 2.5, 3.3)
  - Added `batteryVoltageDivider` configuration field (default: 2.0)
  - New `setVoltageDivider(float ratio)` method for runtime adjustment
  - More accurate ADC-to-voltage mapping calculations
  - Better support for custom hardware configurations

### Improved Battery Monitoring Algorithm
- Enhanced ADC reading with better averaging and scaling
- More accurate voltage calculation: `map(averaged, 0, 4095, 0, (uint16_t)(33 * scale)) + add`
- Better debug logging with averaged values and calculated voltages
- Supports wider range of battery types and voltage divider configurations

## UI Enhancements

### Battery Status Indicator
- **New battery voltage indicator in header**
  - Real-time battery voltage display
  - Dynamic battery icon (full/mid/low states)
  - Visual warning when battery is low
  - Only displays when battery monitoring is enabled

### New SVG Assets
- Added `bat-full-svg.svg` - Battery full icon
- Added `bat-mid-svg.svg` - Battery medium icon
- Added `bat-low-svg.svg` - Battery low icon
- Added `wifi-svg.svg` - WiFi connection icon
- Added `logo-black.png` and `logo-white.png` - FPVGate logos
- All SVG assets optimized for minimal file size

### Visual Polish
- Improved connection status indicator styling
- Enhanced header layout with battery and WiFi indicators
- Better responsive design for mobile devices
- Refined color scheme for battery states (green/yellow/red)

## Technical Improvements

### Configuration System
- Added battery voltage divider persistence to config v6
- New configuration methods: `getBatteryVoltageDivider()` and `setBatteryVoltageDivider()`
- Battery monitoring configuration saved across reboots
- Better configuration validation and error handling

### Frontend Updates
- Enhanced battery monitoring JavaScript with proper uint16 handling
- Improved battery voltage polling with error handling
- Better WebSocket message handling for battery updates
- Reduced console spam from battery monitoring requests

### Backend Updates
- Updated webserver battery voltage endpoint to return uint16 values
- Improved battery alarm logic with new voltage precision
- Better integration between battery monitor and config system
- More robust error handling for battery-related operations

## Modified Files

### Backend (lib/)
- `lib/BATTERY/battery.cpp` - Changed voltage type to uint16, added float scale parameter
- `lib/BATTERY/battery.h` - Updated method signatures, added setVoltageDivider()
- `lib/CONFIG/config.cpp` - Added batteryVoltageDivider configuration field
- `lib/CONFIG/config.h` - New configuration methods for voltage divider
- `lib/WEBSERVER/webserver.cpp` - Updated battery voltage endpoints
- `src/main.cpp` - Battery monitor initialization with new parameters
- `lib/VERSION/version.h` - Version bumped to 1.5.6

### Frontend (data/)
- `data/index.html` - Added battery indicator in header, new SVG assets
- `data/script.js` - Enhanced battery monitoring with uint16 support
- `data/style.css` - New styles for battery indicator and icons
- `data/locales/en.json` - Added battery-related translation keys

### New Assets (data/)
- `data/bat-full-svg.svg` - Battery full icon (NEW)
- `data/bat-mid-svg.svg` - Battery medium icon (NEW)
- `data/bat-low-svg.svg` - Battery low icon (NEW)
- `data/wifi-svg.svg` - WiFi connection icon (NEW)
- `data/logo-black.png` - Black logo (NEW)
- `data/logo-white.png` - White logo (NEW)

## Bug Fixes Summary

1. Fixed battery voltage precision limited to 0-255 range
2. Fixed voltage divider scale parameter limited to integer values
3. Fixed inaccurate battery voltage readings on custom hardware
4. Fixed battery voltage debug output not showing actual voltage
5. Fixed battery monitoring configuration not persisting across reboots

## New Features Summary

1. High-precision battery voltage monitoring (uint16 range)
2. Configurable voltage divider ratio (float precision)
3. Visual battery indicator in UI header
4. Dynamic battery icon states (full/mid/low)
5. Enhanced debug output with actual voltage display

## Installation Notes

### Firmware Upload
This release includes firmware for all supported boards:
- **ESP32S3** - Generic ESP32-S3 boards
- **ESP32S3SuperMini** - ESP32-S3 Super Mini boards
- **SeeedXIAOESP32S3** - Seeed Studio XIAO ESP32S3
- **LilyGOTEnergyS3** - LilyGO T-Energy S3
- **LicardoTimerS3** - Licardo Timer S3
- **LicardoTimerC3** - Licardo Timer C3
- **ESP32C3** - Generic ESP32-C3 boards

### Upload Methods
#### Method 1: PlatformIO (Recommended)
```bash
# Upload firmware
pio run -e [board_name] -t upload --upload-port [COM_PORT]

# Upload filesystem
pio run -e [board_name] -t uploadfs --upload-port [COM_PORT]
```

#### Method 2: ESP Flash Download Tool
1. Download ESP Flash Download Tool from Espressif
2. Select your chip type (ESP32-S3 or ESP32-C3)
3. Add binaries:
   - Firmware: `0x10000` - `FPVGate_v1.5.6_[board]_firmware.bin`
   - Filesystem: `0x410000` - `FPVGate_v1.5.6_[board]_filesystem.bin`
4. Click "Start" to flash

#### Method 3: esptool.py
```bash
# Erase flash first (recommended)
esptool.py --chip esp32s3 --port [COM_PORT] erase_flash

# Flash firmware and filesystem
esptool.py --chip esp32s3 --port [COM_PORT] --baud 460800 write_flash 0x10000 FPVGate_v1.5.6_[board]_firmware.bin 0x410000 FPVGate_v1.5.6_[board]_filesystem.bin
```

### Configuration Migration
- **Battery monitoring configuration will be preserved** on upgrade
- **New voltage divider setting defaults to 2.0** (standard 2:1 divider)
- If you have custom hardware, adjust `batteryVoltageDivider` in settings
- No action required for standard hardware configurations

## Breaking Changes

None - this release is fully backwards compatible with v1.5.5.

## Tested On

- Seeed XIAO ESP32S3
- ESP32-S3 DevKitC-1
- ESP32-C3 DevKitM-1
- LilyGO T-Energy S3
- Battery monitoring verified with various voltage divider ratios
- Configuration persistence tested across reboots

## Credits

- FPVGate community for testing and feedback on battery monitoring precision
- Contributors who reported voltage reading inaccuracies

---

**Full Changelog:** https://github.com/LouisHitchcock/FPVGate/compare/v1.5.5...v1.5.6
