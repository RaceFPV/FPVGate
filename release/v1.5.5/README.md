# FPVGate v1.5.5 Release Binaries

## Installation Instructions

### Firmware Files
The firmware files (`*_firmware.bin`) contain the main program code.

### Filesystem Files  
The filesystem files (`*_filesystem.bin`) contain the web interface and configuration.

### Board Selection
Choose the binary that matches your hardware:
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
   - Firmware: `0x10000` - `FPVGate_v1.5.5_[board]_firmware.bin`
   - Filesystem: `0x410000` - `FPVGate_v1.5.5_[board]_filesystem.bin`
4. Click "Start" to flash

#### Method 3: esptool.py
```bash
# Erase flash first (recommended)
esptool.py --chip esp32s3 --port [COM_PORT] erase_flash

# Flash firmware and filesystem
esptool.py --chip esp32s3 --port [COM_PORT] --baud 460800 write_flash 0x10000 FPVGate_v1.5.5_[board]_firmware.bin 0x410000 FPVGate_v1.5.5_[board]_filesystem.bin
```

## Audio Files (Separate Download)

### Matilda Voice (NEW in v1.5.5)
- Download `sounds_matilda.zip` from release assets
- Extract to SD card root directory
- Select "Matilda" voice in FPVGate settings

### Other Voices
- `sounds_default` (Sarah) - Included in filesystem
- `sounds_rachel` - Download separately
- `sounds_adam` - Download separately
- `sounds_antoni` - Download separately

## What's New in v1.5.5

See `CHANGELOG.md` for full details.

### Highlights
- ✨ New Matilda voice (ElevenLabs - Warm Female)
- ✨ System-based VTX band selection (Analog/HDZero/Walksnail/DJI)
- ✨ Audio crossfade for seamless transitions
- 🐛 Fixed decimal pronunciation (4.04 says "four point zero four")
- 🐛 Fixed battery voltage polling spam
- 🐛 Fixed SD card audio serving

## Support

- **Documentation:** https://fpvgate.com/docs
- **GitHub:** https://github.com/LouisHitchcock/FPVGate
- **Issues:** https://github.com/LouisHitchcock/FPVGate/issues

---

**Version:** v1.5.5  
**Release Date:** January 30, 2026
