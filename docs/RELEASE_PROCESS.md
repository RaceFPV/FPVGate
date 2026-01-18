# FPVGate Release Process Guide

This document describes the complete process for creating and publishing FPVGate releases.

## Release Structure

Each release must follow this exact structure to work with the web flasher at https://fpvgate.xyz:

```
release/vX.Y.Z/
├── boards/                          # For web flasher (fpvgate.xyz)
│   ├── ESP32S3-8MB-bootloader.bin
│   ├── ESP32S3-8MB-firmware.bin
│   ├── ESP32S3-8MB-littlefs.bin
│   ├── ESP32S3-8MB-partitions.bin
│   ├── ESP32S3SuperMini-4MB-bootloader.bin
│   ├── ESP32S3SuperMini-4MB-firmware.bin
│   ├── ESP32S3SuperMini-4MB-littlefs.bin
│   └── ESP32S3SuperMini-4MB-partitions.bin
├── ESP32S3-bootloader.bin            # For GitHub release downloads
├── ESP32S3-firmware.bin
├── ESP32S3-littlefs.bin
├── ESP32S3-partitions.bin
├── ESP32S3-FLASH_INSTRUCTIONS.txt
├── ESP32S3SuperMini-bootloader.bin
├── ESP32S3SuperMini-firmware.bin
├── ESP32S3SuperMini-littlefs.bin
├── ESP32S3SuperMini-partitions.bin
├── ESP32S3SuperMini-FLASH_INSTRUCTIONS.txt
└── RELEASE_NOTES.md
```

## Why Two Sets of Binaries?

### 1. `boards/` Directory
- **Purpose**: Used by the web flasher at https://fpvgate.xyz
- **Naming**: Must use `-8MB-` and `-4MB-` suffixes to indicate flash size
- **URL Pattern**: `https://fpvgate.xyz/firmware/vX.Y.Z/boards/ESP32S3-8MB-bootloader.bin`
- **Must be committed to repository**: The website pulls files from the GitHub repository, NOT release assets

### 2. Root Level Files
- **Purpose**: Individual downloads for users flashing manually
- **Naming**: Simpler names for easier identification
- **Distribution**: GitHub release assets and optional zip files

## Automated Release Process

### Step 1: Update Version Number

Edit `lib/VERSION/version.h`:
```cpp
#define FPVGATE_VERSION "X.Y.Z"
```

### Step 2: Update CHANGELOG.md

Add new section at the top:
```markdown
## [X.Y.Z] - YYYY-MM-DD

### Added/Fixed/Changed
- Description of changes
```

### Step 3: Create Release Notes

Create `release/vX.Y.Z/RELEASE_NOTES.md` following the template from previous releases.

### Step 4: Tag and Push

```bash
git add -A
git commit -m "Release vX.Y.Z: <brief description>"
git tag -a vX.Y.Z -m "FPVGate vX.Y.Z - <brief description>"
git push origin main
git push origin vX.Y.Z
```

The GitHub Actions workflow will automatically:
1. Build firmware for all boards
2. Create properly named binaries
3. Create GitHub release with assets
4. Commit binaries to `release/vX.Y.Z/boards/` directory

### Step 5: Verify Web Flasher

After the workflow completes:
1. Visit https://fpvgate.xyz/flasher.html
2. Select the new version
3. Verify that all binaries download correctly

## Manual Release Process (Fallback)

If the automated process fails, follow these steps:

### 1. Build Firmware

```bash
# ESP32S3 DevKitC-1 (8MB)
pio run -e ESP32S3
pio run -e ESP32S3 -t buildfs

# ESP32S3 Super Mini (4MB)
pio run -e ESP32S3SuperMini
pio run -e ESP32S3SuperMini -t buildfs
```

### 2. Copy Binaries to Release Directory

```bash
# Create release directory
mkdir -p release/vX.Y.Z/boards

# ESP32S3 (8MB) - Root level
cp .pio/build/ESP32S3/bootloader.bin release/vX.Y.Z/ESP32S3-bootloader.bin
cp .pio/build/ESP32S3/firmware.bin release/vX.Y.Z/ESP32S3-firmware.bin
cp .pio/build/ESP32S3/littlefs.bin release/vX.Y.Z/ESP32S3-littlefs.bin
cp .pio/build/ESP32S3/partitions.bin release/vX.Y.Z/ESP32S3-partitions.bin

# ESP32S3 (8MB) - boards/ directory
cp .pio/build/ESP32S3/bootloader.bin release/vX.Y.Z/boards/ESP32S3-8MB-bootloader.bin
cp .pio/build/ESP32S3/firmware.bin release/vX.Y.Z/boards/ESP32S3-8MB-firmware.bin
cp .pio/build/ESP32S3/littlefs.bin release/vX.Y.Z/boards/ESP32S3-8MB-littlefs.bin
cp .pio/build/ESP32S3/partitions.bin release/vX.Y.Z/boards/ESP32S3-8MB-partitions.bin

# ESP32S3 Super Mini (4MB) - Root level
cp .pio/build/ESP32S3SuperMini/bootloader.bin release/vX.Y.Z/ESP32S3SuperMini-bootloader.bin
cp .pio/build/ESP32S3SuperMini/firmware.bin release/vX.Y.Z/ESP32S3SuperMini-firmware.bin
cp .pio/build/ESP32S3SuperMini/littlefs.bin release/vX.Y.Z/ESP32S3SuperMini-littlefs.bin
cp .pio/build/ESP32S3SuperMini/partitions.bin release/vX.Y.Z/ESP32S3SuperMini-partitions.bin

# ESP32S3 Super Mini (4MB) - boards/ directory
cp .pio/build/ESP32S3SuperMini/bootloader.bin release/vX.Y.Z/boards/ESP32S3SuperMini-4MB-bootloader.bin
cp .pio/build/ESP32S3SuperMini/firmware.bin release/vX.Y.Z/boards/ESP32S3SuperMini-4MB-firmware.bin
cp .pio/build/ESP32S3SuperMini/littlefs.bin release/vX.Y.Z/boards/ESP32S3SuperMini-4MB-littlefs.bin
cp .pio/build/ESP32S3SuperMini/partitions.bin release/vX.Y.Z/boards/ESP32S3SuperMini-4MB-partitions.bin
```

### 3. Create Flash Instructions

Create `release/vX.Y.Z/ESP32S3-FLASH_INSTRUCTIONS.txt` and `ESP32S3SuperMini-FLASH_INSTRUCTIONS.txt` following the template from v1.5.3.

### 4. Commit Binaries to Repository

```bash
# IMPORTANT: Binaries in boards/ must be committed to work with web flasher
git add -f release/vX.Y.Z/boards/*.bin
git add release/vX.Y.Z/RELEASE_NOTES.md
git add release/vX.Y.Z/*FLASH_INSTRUCTIONS.txt
git commit -m "Add vX.Y.Z release binaries for web flasher"
git push origin main
```

### 5. Upload to GitHub Release

```bash
gh release upload vX.Y.Z \
  release/vX.Y.Z/ESP32S3-*.bin \
  release/vX.Y.Z/ESP32S3-FLASH_INSTRUCTIONS.txt \
  release/vX.Y.Z/ESP32S3SuperMini-*.bin \
  release/vX.Y.Z/ESP32S3SuperMini-FLASH_INSTRUCTIONS.txt \
  release/vX.Y.Z/RELEASE_NOTES.md
```

## Hotfix Process (Silent Update)

For bug fixes that don't warrant a new version number:

### 1. Fix the Bug

Make code changes and test thoroughly.

### 2. Rebuild Affected Binaries

Only rebuild what changed (usually just littlefs.bin for web interface fixes):

```bash
pio run -e ESP32S3 -t buildfs
pio run -e ESP32S3SuperMini -t buildfs
```

### 3. Update Release Binaries

```bash
# Update boards/ directory (for web flasher)
cp .pio/build/ESP32S3/littlefs.bin release/vX.Y.Z/boards/ESP32S3-8MB-littlefs.bin
cp .pio/build/ESP32S3SuperMini/littlefs.bin release/vX.Y.Z/boards/ESP32S3SuperMini-4MB-littlefs.bin

# Update root level (for downloads)
cp .pio/build/ESP32S3/littlefs.bin release/vX.Y.Z/ESP32S3-littlefs.bin
cp .pio/build/ESP32S3SuperMini/littlefs.bin release/vX.Y.Z/ESP32S3SuperMini-littlefs.bin
```

### 4. Commit and Upload

```bash
git add -f release/vX.Y.Z/boards/*.bin
git add -f release/vX.Y.Z/*.bin
git commit -m "Hotfix: <brief description> for vX.Y.Z"
git push origin main

# Replace GitHub release assets
gh release upload vX.Y.Z \
  release/vX.Y.Z/ESP32S3-littlefs.bin \
  release/vX.Y.Z/ESP32S3SuperMini-littlefs.bin \
  --clobber
```

## Web Flasher Integration

The web flasher at https://fpvgate.xyz expects:

### URL Pattern
```
https://raw.githubusercontent.com/LouisHitchcock/FPVGate/main/release/vX.Y.Z/boards/BOARD-FLASH_SIZE-TYPE.bin
```

Or through the website proxy:
```
https://fpvgate.xyz/firmware/vX.Y.Z/boards/BOARD-FLASH_SIZE-TYPE.bin
```

### File Names Must Match
- `ESP32S3-8MB-bootloader.bin`
- `ESP32S3-8MB-firmware.bin`
- `ESP32S3-8MB-littlefs.bin`
- `ESP32S3-8MB-partitions.bin`
- `ESP32S3SuperMini-4MB-bootloader.bin`
- `ESP32S3SuperMini-4MB-firmware.bin`
- `ESP32S3SuperMini-4MB-littlefs.bin`
- `ESP32S3SuperMini-4MB-partitions.bin`

### Testing Checklist

After releasing:
- [ ] Visit https://fpvgate.xyz/flasher.html
- [ ] Select new version from dropdown
- [ ] Select ESP32-S3 DevKitC-1 (8MB) - verify all 4 files are found
- [ ] Select ESP32-S3 Super Mini (4MB) - verify all 4 files are found
- [ ] Test flashing on actual hardware
- [ ] Verify GitHub release has all individual binaries as assets
- [ ] Check that release notes are correct

## Troubleshooting

### Web Flasher Can't Find Binaries
- **Problem**: 404 errors when trying to flash
- **Solution**: Ensure binaries are committed to `release/vX.Y.Z/boards/` in the repository
- **Verify**: Check https://github.com/LouisHitchcock/FPVGate/tree/main/release/vX.Y.Z/boards

### Wrong File Names
- **Problem**: Binaries have wrong naming convention
- **Solution**: Rename files to match exact pattern above (case-sensitive)
- **Key**: Must include `-8MB-` or `-4MB-` in boards/ directory files

### Release Assets Not Uploading
- **Problem**: GitHub Actions fails to create release
- **Solution**: Check workflow logs, ensure tag is pushed, verify GITHUB_TOKEN permissions

## Version Numbering

Follow Semantic Versioning (SemVer):
- **Major (X.0.0)**: Breaking changes, major new features
- **Minor (x.Y.0)**: New features, backwards compatible
- **Patch (x.y.Z)**: Bug fixes, no new features

Examples:
- v1.5.3 → v1.5.4 (bug fix)
- v1.5.4 → v1.6.0 (new feature)
- v1.6.0 → v2.0.0 (major rewrite/breaking changes)
