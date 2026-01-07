================================================================================
                          FPVGate v1.5.1
                    Personal FPV Lap Timer
================================================================================

QUICK START
-----------

1. FLASH FIRMWARE

   Option A - Web Flasher (Easiest):
   Visit: https://fpvgate.xyz/flasher.html
   Follow the on-screen instructions to flash directly from your browser.

   Option B - Manual Flash:
   esptool.py --chip esp32s3 --port COM3 write_flash -z \
     0x0 bootloader.bin \
     0x8000 partitions.bin \
     0x10000 firmware.bin \
     0x410000 littlefs.bin

   Replace COM3 with your port (/dev/ttyUSB0 on Linux/Mac)

2. PREPARE SD CARD
   - Format as FAT32
   - Extract sd_card_contents.zip to the root of your SD card
   - Insert SD card into ESP32-S3

3. CONNECT
   - Power on your FPVGate device
   - Connect to WiFi: FPVGate_XXXX
   - Password: fpvgate1
   - Open browser to: http://fpvgate.local or http://192.168.4.1

4. CONFIGURE
   - Set your VTx frequency
   - Calibrate RSSI thresholds
   - Configure race settings

5. RACE
   - Fly through the gate
   - Lap times appear in real-time
   - Voice announcements for each lap


WHAT'S NEW IN v1.5.1
--------------------

Device Domain Change:
- Device now accessible at http://fpvgate.local (changed from fpvgate.xyz)
- Fixes conflicts with public website
- Improved captive portal experience

New Hardware Support:
- LilyGO T-Energy S3 with battery monitoring
- ESP32-C3 with USB CDC support
- ESP32-S3 Super Mini (4MB Flash)

Bug Fixes:
- Fixed captive portal domain conflicts
- Fixed TTS translation keys for multi-language support
- Fixed case-sensitive includes for cross-platform compatibility

For full release notes, see RELEASE_NOTES.md


SUPPORTED HARDWARE
------------------

Recommended:
- ESP32-S3 DevKitC-1 (8MB Flash)

Also Supported:
- ESP32-S3 Super Mini (4MB Flash)
- LilyGO T-Energy S3 (battery powered)
- ESP32-C3 (budget option)

Required Components:
- RX5808 Module (SPI modded)
- MicroSD Card (FAT32 formatted)
- 5V Power Supply (or battery for T-Energy S3)


IMPORTANT NOTES
---------------

Device URL Change:
If you used the old URL, please update your bookmarks:
  Old: http://fpvgate.xyz
  New: http://fpvgate.local or http://192.168.4.1

License:
FPVGate is licensed under CC BY-NC-SA 4.0
- Free for personal and educational use
- Modifications allowed with attribution
- Commercial use requires permission


TROUBLESHOOTING
---------------

Device Not Found:
- Ensure USB cable supports data transfer
- Hold BOOT button while connecting USB
- Try different USB port
- Check device manager for COM port

Can't Connect to WiFi:
- Verify WiFi network name: FPVGate_XXXX
- Password is: fpvgate1
- Disable mobile data on phone
- Forget and reconnect to network

Can't Access Web Interface:
- Try http://fpvgate.local first
- If that fails, try http://192.168.4.1
- Clear browser cache
- Try different browser


LINKS
-----

Public Website:  https://fpvgate.xyz
Web Flasher:     https://fpvgate.xyz/flasher.html
Documentation:   https://github.com/LouisHitchcock/FPVGate/tree/main/docs
Issue Tracker:   https://github.com/LouisHitchcock/FPVGate/issues
Discussions:     https://github.com/LouisHitchcock/FPVGate/discussions


================================================================================
Made with care for the FPV community
FPVGate is open source (CC BY-NC-SA 4.0) and built by pilots, for pilots.
================================================================================
