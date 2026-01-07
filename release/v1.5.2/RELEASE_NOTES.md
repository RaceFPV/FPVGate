# FPVGate v1.5.2 Release Notes

**Release Date:** January 7, 2026  
**Type:** Minor Release - Bug Fixes & Enhancements

## What's New in v1.5.2

### UI Enhancements
- **Version Display in Footer**: Device interface now shows firmware version (e.g., "FPVGate v1.5.2") in the footer
- **Website Badge**: Added prominent website badge (https://fpvgate.xyz) to README for easy access

### Bug Fixes
- **Fixed Initial Tab Display**: Resolved issue where Race History content appeared on page load instead of being hidden
- **Fixed Translation Keys**: Corrected lap timeline labels that were showing "Lap {n}" instead of proper lap numbers

### Documentation Updates
- **Power Supply Recommendations**: Updated all documentation to recommend "USB power or any 5V source" instead of specific battery configurations
- **Wiring Diagrams**: Reorganized wiring diagrams into separate files for each board type
  - ESP32-S3 DevKitC-1 (8MB Flash)
  - ESP32-S3 Super Mini (4MB Flash)  
  - ESP32-C3
  - LilyGO T-Energy S3
- **Website Integration**: Added dynamic board selector on fpvgate.xyz documentation page for wiring diagrams

## Technical Changes

### Version Management
- Created centralized version header (`lib/VERSION/version.h`)
- Added `/version` API endpoint for programmatic version checking
- JavaScript function to fetch and display version dynamically

### Code Quality
- Improved CSS for footer styling with better theming support
- Fixed inline style issues for proper tab initialization
- Enhanced markdown rendering for website wiring diagrams

## Supported Hardware

- **ESP32-S3 DevKitC-1** (8MB Flash) - Recommended
- **ESP32-S3 Super Mini** (4MB Flash) - Compact
- **ESP32-C3** - Expert mode
- **LilyGO T-Energy S3** - Expert mode with integrated battery

## Installation

### Web Flasher (Recommended)
Visit **https://fpvgate.xyz/flasher.html** and follow the on-screen instructions.

### Command Line
Download board-specific binaries from the [releases page](https://github.com/LouisHitchcock/FPVGate/releases) and flash:
```bash
esptool.py --chip esp32s3 --port COM3 write_flash -z \
  0x0 [BOARD]-bootloader.bin \
  0x8000 [BOARD]-partitions.bin \
  0x10000 [BOARD]-firmware.bin \
  0x410000 [BOARD]-littlefs.bin
```

## Upgrade Notes

- This is a direct upgrade from v1.5.1
- No configuration reset required
- All existing settings will be preserved

## Known Issues

None reported for this release.

## Contributors

- Louis Hitchcock (@LouisHitchcock)

## Links

- **Website**: https://fpvgate.xyz
- **Documentation**: https://github.com/LouisHitchcock/FPVGate/tree/main/docs
- **Issues**: https://github.com/LouisHitchcock/FPVGate/issues
