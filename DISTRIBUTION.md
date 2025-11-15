# Live Captions - Distribution Guide

## What's Included in the DMG

The `LiveCaptions-1.0.dmg` contains:
- **LiveCaptions.app** - Fully self-contained application bundle with all dependencies
- **SETUP_MACOS.txt** - Setup instructions for users
- **Applications symlink** - For easy drag-and-drop installation

## Dependencies Status

### ✅ Bundled (Users don't need to install)
- All GTK4/libadwaita libraries
- ONNX Runtime
- April ASR model (321MB - embedded in app)
- All Homebrew dependencies (glib, cairo, pango, etc.)

### ❌ NOT Bundled (Users must install separately)
- **BlackHole audio driver** - Required ONLY for Desktop Audio mode
  - Microphone mode works without it
  - This is a system-level audio driver that cannot be bundled
  - Users need to install from: https://github.com/ExistentialAudio/BlackHole
  - Install via: `brew install blackhole-2ch`

## User Installation Steps

### Basic Installation (Microphone Mode Only)
1. Download `LiveCaptions-1.0.dmg`
2. Open the DMG
3. Drag `Live Captions.app` to Applications folder
4. Open the app
5. Toggle microphone and start speaking

### Full Installation (Desktop Audio + Microphone)
1. Install BlackHole:
   ```bash
   brew install blackhole-2ch
   ```
2. Set up Multi-Output Device (see SETUP_MACOS.txt for details)
3. Install Live Captions from DMG
4. Toggle Desktop Audio mode to capture system sound

## What Fresh Machines Need

### Scenario 1: User only wants microphone captions
- **Nothing extra needed** - Just install the DMG

### Scenario 2: User wants desktop audio captions (e.g., from Zoom, YouTube)
- **Must install BlackHole** and configure Multi-Output Device
- See SETUP_MACOS.txt for step-by-step guide

## Graceful Degradation

If BlackHole is not installed:
- Desktop Audio mode will print a warning and fall back to microphone
- Microphone mode continues to work normally
- App does not crash - it handles the missing driver gracefully

## Testing on Fresh Machines

To test the DMG on a clean system:
1. Copy `LiveCaptions-1.0.dmg` to a fresh Mac
2. Double-click to mount
3. Drag app to Applications
4. Open app
5. Test microphone mode (should work immediately)
6. Test desktop audio mode (should show BlackHole warning)
7. Install BlackHole and configure Multi-Output Device
8. Test desktop audio mode again (should work)

## Support Documentation

Users should read `SETUP_MACOS.txt` included in the DMG for:
- BlackHole installation instructions
- Multi-Output Device configuration
- Troubleshooting common issues
