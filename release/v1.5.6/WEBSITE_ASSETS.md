# FPVGate v1.5.6 - Website Release Assets

## Release Information
- **Version:** 1.5.6
- **Release Date:** February 1, 2026
- **GitHub Release:** https://github.com/LouisHitchcock/FPVGate/releases/tag/v1.5.6

## Firmware Binaries (14 files)

### ESP32-S3 Boards (7 firmware + 7 filesystem)

**Firmware Files:**
1. `FPVGate_v1.5.6_ESP32S3_firmware.bin` (1.18 MB)
2. `FPVGate_v1.5.6_ESP32S3SuperMini_firmware.bin` (1.18 MB)
3. `FPVGate_v1.5.6_SeeedXIAOESP32S3_firmware.bin` (1.17 MB)
4. `FPVGate_v1.5.6_LilyGOTEnergyS3_firmware.bin` (1.17 MB)
5. `FPVGate_v1.5.6_LicardoTimerS3_firmware.bin` (1.18 MB)
6. `FPVGate_v1.5.6_LicardoTimerC3_firmware.bin` (1.26 MB)
7. `FPVGate_v1.5.6_ESP32C3_firmware.bin` (1.26 MB)

**Filesystem Files:**
1. `FPVGate_v1.5.6_ESP32S3_filesystem.bin` (1.00 MB)
2. `FPVGate_v1.5.6_ESP32S3SuperMini_filesystem.bin` (1.38 MB)
3. `FPVGate_v1.5.6_SeeedXIAOESP32S3_filesystem.bin` (1.00 MB)
4. `FPVGate_v1.5.6_LilyGOTEnergyS3_filesystem.bin` (1.00 MB)
5. `FPVGate_v1.5.6_LicardoTimerS3_filesystem.bin` (1.00 MB)
6. `FPVGate_v1.5.6_LicardoTimerC3_filesystem.bin` (1.38 MB)
7. `FPVGate_v1.5.6_ESP32C3_filesystem.bin` (1.38 MB)

## Audio Assets

**SD Card Contents:**
- `sd_card.zip` (15.3 MB)
- Contains default voice files and SD card structure
- Includes alternative voice packs (Matilda, Rachel, Adam, Antoni)
- Extract to SD card root directory

## Documentation

1. `README.md` - Installation instructions and board selection guide
2. `CHANGELOG.md` - Detailed changelog with all improvements and fixes

## Website Download Links Format

For the FPVGate website, use these download link formats:

```markdown
### Firmware Files

#### ESP32-S3 Boards
- [ESP32S3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_ESP32S3_firmware.bin)
- [ESP32S3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_ESP32S3_filesystem.bin)

- [ESP32S3 Super Mini Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_ESP32S3SuperMini_firmware.bin)
- [ESP32S3 Super Mini Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_ESP32S3SuperMini_filesystem.bin)

- [Seeed XIAO ESP32S3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_SeeedXIAOESP32S3_firmware.bin)
- [Seeed XIAO ESP32S3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_SeeedXIAOESP32S3_filesystem.bin)

- [LilyGO T-Energy S3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_LilyGOTEnergyS3_firmware.bin)
- [LilyGO T-Energy S3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_LilyGOTEnergyS3_filesystem.bin)

- [Licardo Timer S3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_LicardoTimerS3_firmware.bin)
- [Licardo Timer S3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_LicardoTimerS3_filesystem.bin)

#### ESP32-C3 Boards
- [Licardo Timer C3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_LicardoTimerC3_firmware.bin)
- [Licardo Timer C3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_LicardoTimerC3_filesystem.bin)

- [ESP32C3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_ESP32C3_firmware.bin)
- [ESP32C3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/FPVGate_v1.5.6_ESP32C3_filesystem.bin)

### Audio Assets
- [SD Card Contents (15.3 MB)](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.6/sd_card.zip)
```

## Key Features to Highlight on Website

### 🔋 New in v1.5.6: High-Precision Battery Monitoring
- Upgraded from 8-bit to 16-bit voltage precision
- Accurate readings to 0.1V resolution (e.g., 4.2V)
- Configurable voltage divider for custom hardware
- Visual battery indicator in UI header

### 🎨 UI Improvements
- Real-time battery voltage display with dynamic icons
- Battery states: Full (green), Medium (yellow), Low (red)
- WiFi connection indicator in header
- Enhanced visual feedback for system status

### 🔧 Technical Enhancements
- Float precision for voltage divider configuration
- Battery voltage persists across reboots
- Better ADC-to-voltage mapping algorithm
- Improved debug output with actual voltage values

### 🐛 Bug Fixes
- Fixed battery voltage precision (8-bit → 16-bit)
- Fixed voltage divider configuration for custom hardware
- Fixed battery monitoring persistence across reboots
- Improved battery alarm accuracy

## Website Changelog Summary

```
# FPVGate v1.5.6 (February 1, 2026)

## 🔋 Battery Monitoring
- **High-Precision Voltage Monitoring** - 16-bit resolution (0-65535 range)
- **Configurable Voltage Divider** - Float precision for custom hardware
- **Visual Battery Indicator** - Real-time display in UI header
- **Dynamic Battery Icons** - Full/Medium/Low states with color coding

## 🎨 UI Enhancements
- Battery voltage indicator in header
- WiFi connection status indicator
- New SVG battery and WiFi icons
- Improved mobile responsiveness

## 🐛 Bug Fixes
- Fixed 8-bit voltage precision limitation
- Fixed voltage divider integer-only configuration
- Fixed battery monitoring persistence
- Improved battery alarm accuracy

## 🚀 Improvements
- More accurate battery voltage readings
- Better support for custom hardware
- Enhanced debug logging
- Optimized ADC reading algorithm

[View Full Changelog](https://github.com/LouisHitchcock/FPVGate/releases/tag/v1.5.6)
```

## File Size Summary

**Total Release Size:** ~33 MB
- Firmware binaries: ~8.5 MB (7 files)
- Filesystem binaries: ~8.0 MB (7 files)
- SD card contents: ~15.3 MB
- Documentation: ~20 KB

## Installation Instructions for Website

```markdown
### Quick Start

1. **Download** the firmware and filesystem for your board
2. **Flash** using PlatformIO, ESP Flash Tool, or esptool
3. **Download** SD card contents (optional, for alternative voices)
4. **Extract** SD card zip to SD card root
5. **Configure** battery voltage divider if using custom hardware

### Detailed Instructions
See [Installation Guide](https://github.com/LouisHitchcock/FPVGate/releases/tag/v1.5.6)
```

## Migration Notes from v1.5.5

- **Fully Backwards Compatible** - No breaking changes
- **Automatic Configuration Migration** - Battery settings preserved
- **New Battery Features** - Automatically available after upgrade
- **No Manual Intervention Required** - Unless using custom hardware
