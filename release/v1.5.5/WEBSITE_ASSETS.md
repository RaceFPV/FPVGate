# FPVGate v1.5.5 - Website Release Assets

## Release Information
- **Version:** 1.5.5
- **Release Date:** January 30, 2026
- **GitHub Release:** https://github.com/LouisHitchcock/FPVGate/releases/tag/v1.5.5

## Firmware Binaries (14 files)

### ESP32-S3 Boards (7 firmware + 7 filesystem)

**Firmware Files:**
1. `FPVGate_v1.5.5_ESP32S3_firmware.bin` (1.18 MB)
2. `FPVGate_v1.5.5_ESP32S3SuperMini_firmware.bin` (1.18 MB)
3. `FPVGate_v1.5.5_SeeedXIAOESP32S3_firmware.bin` (1.17 MB)
4. `FPVGate_v1.5.5_LilyGOTEnergyS3_firmware.bin` (1.17 MB)
5. `FPVGate_v1.5.5_LicardoTimerS3_firmware.bin` (1.18 MB)
6. `FPVGate_v1.5.5_LicardoTimerC3_firmware.bin` (1.26 MB)
7. `FPVGate_v1.5.5_ESP32C3_firmware.bin` (1.26 MB)

**Filesystem Files:**
1. `FPVGate_v1.5.5_ESP32S3_filesystem.bin` (1.00 MB)
2. `FPVGate_v1.5.5_ESP32S3SuperMini_filesystem.bin` (1.38 MB)
3. `FPVGate_v1.5.5_SeeedXIAOESP32S3_filesystem.bin` (1.00 MB)
4. `FPVGate_v1.5.5_LilyGOTEnergyS3_filesystem.bin` (1.00 MB)
5. `FPVGate_v1.5.5_LicardoTimerS3_filesystem.bin` (1.00 MB)
6. `FPVGate_v1.5.5_LicardoTimerC3_filesystem.bin` (1.38 MB)
7. `FPVGate_v1.5.5_ESP32C3_filesystem.bin` (1.38 MB)

## Audio Assets

**Matilda Voice Pack:**
- `sounds_matilda.zip` (3.18 MB)
- Contains 214 pre-recorded audio files
- Requires SD card installation
- ElevenLabs Matilda voice (Warm Female)

## Documentation

1. `README.md` - Installation instructions and board selection guide
2. `CHANGELOG.md` - Detailed changelog with all improvements and fixes

## Website Download Links Format

For the FPVGate website, use these download link formats:

```markdown
### Firmware Files

#### ESP32-S3 Boards
- [ESP32S3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_ESP32S3_firmware.bin)
- [ESP32S3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_ESP32S3_filesystem.bin)

- [ESP32S3 Super Mini Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_ESP32S3SuperMini_firmware.bin)
- [ESP32S3 Super Mini Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_ESP32S3SuperMini_filesystem.bin)

- [Seeed XIAO ESP32S3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_SeeedXIAOESP32S3_firmware.bin)
- [Seeed XIAO ESP32S3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_SeeedXIAOESP32S3_filesystem.bin)

- [LilyGO T-Energy S3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_LilyGOTEnergyS3_firmware.bin)
- [LilyGO T-Energy S3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_LilyGOTEnergyS3_filesystem.bin)

- [Licardo Timer S3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_LicardoTimerS3_firmware.bin)
- [Licardo Timer S3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_LicardoTimerS3_filesystem.bin)

#### ESP32-C3 Boards
- [Licardo Timer C3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_LicardoTimerC3_firmware.bin)
- [Licardo Timer C3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_LicardoTimerC3_filesystem.bin)

- [ESP32C3 Firmware](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_ESP32C3_firmware.bin)
- [ESP32C3 Filesystem](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/FPVGate_v1.5.5_ESP32C3_filesystem.bin)

### Audio Packs
- [Matilda Voice Pack (3.18 MB)](https://github.com/LouisHitchcock/FPVGate/releases/download/v1.5.5/sounds_matilda.zip)
```

## Key Features to Highlight on Website

### 🎙️ New in v1.5.5: Matilda Voice
- Premium ElevenLabs voice (Warm Female)
- 214 pre-recorded race announcements
- Natural, professional sound quality
- SD card installation required

### 🎛️ System-Based VTX Selection
- Intuitive hierarchical band selection
- Four systems: Analog, HDZero, Walksnail, DJI
- Auto-filters bands by system
- Prevents incorrect frequency selection

### 🔊 Audio Improvements
- Seamless audio transitions (30ms crossfade)
- Fixed decimal pronunciation (4.04 = "four point zero four")
- Eliminated battery monitoring spam
- More natural, human-like speech

### 🐛 Critical Bug Fixes
- Battery voltage polling no longer causes connection drops
- SD card audio serving works correctly
- Band/channel selectors stay synchronized
- Proper decimal time announcements

## Website Changelog Summary

```
# FPVGate v1.5.5 (January 30, 2026)

## 🎙️ New Features
- **Matilda Voice** - New ElevenLabs voice option (Warm Female)
- **System-Based VTX Selection** - Choose Analog/HDZero/Walksnail/DJI first
- **Audio Crossfade** - Seamless transitions between numbers

## 🐛 Bug Fixes
- Fixed decimal pronunciation (4.04 now says "four point zero four")
- Fixed battery voltage polling causing console spam
- Fixed SD card audio file serving
- Fixed band/channel selector synchronization

## 🚀 Improvements
- More natural, fluid audio announcements
- Better audio caching and preloading
- Optimized SD card detection
- Enhanced configuration management

[View Full Changelog](https://github.com/LouisHitchcock/FPVGate/releases/tag/v1.5.5)
```

## File Size Summary

**Total Release Size:** ~19.5 MB
- Firmware binaries: ~8.5 MB (7 files)
- Filesystem binaries: ~8.0 MB (7 files)
- Matilda voice pack: ~3.2 MB
- Documentation: ~10 KB

## Installation Instructions for Website

```markdown
### Quick Start

1. **Download** the firmware and filesystem for your board
2. **Flash** using PlatformIO, ESP Flash Tool, or esptool
3. **Download** Matilda voice pack (optional)
4. **Extract** voice pack to SD card root
5. **Select** Matilda voice in settings

### Detailed Instructions
See [Installation Guide](https://github.com/LouisHitchcock/FPVGate/releases/tag/v1.5.5)
```
