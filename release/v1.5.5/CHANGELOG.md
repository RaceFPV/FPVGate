# FPVGate v1.5.5 - Release Notes

**Release Date:** January 30, 2026

## 🎙️ Audio & Voice Announcements

### New Voice Added
- **Added Matilda voice** (ElevenLabs - Warm Female)
  - New voice option available in settings
  - 214 pre-generated audio files for race announcements
  - Requires SD card for audio file storage
  - Voice ID: ZF6FPAbjXT4488VcRRnw

### Audio Playback Improvements
- **Improved audio fluidity** - Reduced gaps between number announcements
  - Added configurable crossfade/overlap system (30ms for numbers, 20ms for "point")
  - Smoother transitions create more natural, human-like speech
  - Example: "13.52" now flows seamlessly instead of "13 [gap] point [gap] 52"

### Critical Audio Fixes
- **Fixed decimal pronunciation with leading zeros**
  - Times like 4.04 now correctly say "four point zero four" (not "four point four")
  - Times like 12.01 now correctly say "twelve point zero one" (not "twelve point one")
  - All 2-digit decimals with leading zeros are now spoken correctly

- **Fixed battery voltage polling spam**
  - Eliminated console spam from battery voltage requests
  - Battery voltage only polled when battery monitoring is enabled
  - Prevents connection drops from excessive failed requests
  - Silent failure mode for better stability

### Voice Generation Tools
- **Updated voice generation script**
  - Added Matilda voice to generation tools
  - Voice-specific output directories (sounds_matilda, sounds_adam, etc.)
  - Easy regeneration of individual corrupted audio files
  - Script now outputs to proper SD card directory structure

## 🎛️ VTX Frequency Selection UI Refactor

### System-Based Band Selection
- **New hierarchical VTX configuration**
  - Added "System" dropdown with 4 options: Analog, HDZero, Walksnail, DJI
  - Band selector dynamically filters to show only relevant bands for selected system
  - Channel selector automatically disables unavailable channels (frequency = 0)
  - Cleaner, more intuitive UI with inline layout

- **Synchronized selectors** across Settings and Calibration tabs
  - Real-time synchronization between settings modal and calibration UI
  - Band/channel configuration persists correctly across reboots
  - RSSI graph displays properly with new selector system

### Band Organization
- **Analog System:** Boscam A, Boscam B, Boscam E, Fatshark, Raceband, Low Raceband
- **HDZero System:** HDZero Race, HDZero Lowband
- **Walksnail System:** Walksnail, Walksnail VLO
- **DJI System:** DJI

## 🛠️ Technical Improvements

### SD Card Audio Serving
- **Fixed SD card audio file access**
  - Changed `#ifdef ESP32S3` to `#ifdef HAS_SD_CARD_SUPPORT` for proper board detection
  - Replaced problematic regex patterns with simple wildcard routes
  - Added explicit routes for each voice directory
  - Routes registered before catch-all to ensure proper priority
  - SD card test endpoint now works correctly at `/storage/sdtest`

### Configuration Management
- Proper band/channel index storage and retrieval
- Fixed variable shadowing in channel availability updates
- Band definitions mapped to systems with proper indexing

### Audio Infrastructure
- Optimized audio caching and preloading
- Improved error handling for missing audio files
- Better fallback mechanisms (ElevenLabs → PiperTTS → Web Speech API)
- Support for both 1-digit and 2-digit decimal pronunciations

## 📝 Modified Files

### Frontend
- `data/index.html` - Added Matilda voice option, system-based band selectors
- `data/script.js` - System-to-band mapping, dynamic channel filtering, battery monitoring fix
- `data/audio-announcer.js` - Audio crossfade, decimal pronunciation fix, Matilda voice support
- `data/locales/en.json` - Translation keys for Matilda voice and system selectors
- `data/calib-styles.css` - New calibration UI styles (NEW FILE)
- `data/vert-osd.css` - Vertical OSD styles (NEW FILE)
- `data/vert-osd.html` - Vertical OSD layout (NEW FILE)

### Backend
- `lib/WEBSERVER/webserver.cpp` - Added Matilda route, fixed SD card audio serving
- `lib/CONFIG/config.h` - Band/channel configuration structure
- `lib/CONFIG/config.cpp` - Configuration save/load improvements
- `lib/VERSION/version.h` - Version bumped to 1.5.5

### Tools
- `tools/generate_voice_files.py` - Added Matilda voice, voice-specific directories

## 🎯 Bug Fixes Summary

1. ✅ Fixed battery voltage polling causing console spam and connection drops
2. ✅ Fixed decimal pronunciation with leading zeros (4.04, 12.01, etc.)
3. ✅ Fixed SD card audio file serving for alternative voices
4. ✅ Fixed band/channel selector synchronization across UI
5. ✅ Fixed audio gaps between number announcements
6. ✅ Fixed variable shadowing in channel availability checks

## 🚀 New Features Summary

1. ✨ Matilda voice (ElevenLabs - Warm Female)
2. ✨ System-based VTX band selection (Analog/HDZero/Walksnail/DJI)
3. ✨ Audio crossfade system for seamless number transitions
4. ✨ Improved audio fluidity and natural speech flow

## 📦 Installation Notes

### Audio Files
- Matilda voice files are **not included** in the firmware binary
- Download `sounds_matilda.zip` separately and copy to SD card root
- Alternative voices (Rachel, Adam, Antoni) also require SD card

### Firmware Upload
- Upload firmware via PlatformIO: `pio run -e [board] -t upload`
- Upload filesystem: `pio run -e [board] -t uploadfs`
- Supported boards: SeeedXIAOESP32S3, ESP32S3, LicardoTimerS3, etc.

### SD Card Setup
- Format SD card as FAT32
- Copy voice directories to SD card root:
  - `sounds_default/` (Sarah - included in firmware)
  - `sounds_matilda/` (download separately)
  - `sounds_rachel/` (download separately)
  - `sounds_adam/` (download separately)
  - `sounds_antoni/` (download separately)

## 🔧 Breaking Changes

None - this release is fully backwards compatible with v1.5.4.

## 📊 Tested On

- Seeed XIAO ESP32S3
- SD card support verified
- Alternative voice playback verified
- Band/channel selection across all VTX systems

## 🙏 Credits

- ElevenLabs for voice synthesis API
- FPVGate community for testing and feedback

---

**Full Changelog:** https://github.com/yourusername/FPVGate/compare/v1.5.4...v1.5.5
