# Live Captions - Icon and DMG Creation Guide

## 📦 Current Status

✅ DMG installer created: `LiveCaptions-1.0.dmg`
⚠️  Using default macOS icon (needs custom icon)

## 🎨 Adding a Custom Icon

### Step 1: Prepare Your Icon Image

Your icon should be:
- **Format**: PNG, JPEG, or any image format
- **Recommended size**: 1024x1024 pixels
- **Theme**: Interview assistant or microphone/caption related

### Step 2: Convert to .icns Format

Once you have your icon image (let's say `my_icon.png`):

```bash
./create_icon.sh my_icon.png
```

This will create `LiveCaptions.icns` with all required sizes.

### Step 3: Rebuild the App

```bash
./package.sh
```

The script will automatically include the icon in the app bundle.

### Step 4: Create New DMG

```bash
./create_dmg.sh
```

## 📀 DMG File Details

**File**: `LiveCaptions-1.0.dmg`
**Size**: ~600MB (includes all dependencies)
**Contents**:
- Live Captions.app (fully self-contained)
- Applications symlink (for easy installation)

## 🚀 Installation Instructions (for end users)

1. Double-click `LiveCaptions-1.0.dmg`
2. Drag "Live Captions" to the Applications folder
3. Launch from Applications or Spotlight

## 🛠️ Development Workflow

### Full rebuild and package:
```bash
# 1. Build the app
ninja -C builddir

# 2. Package into .app bundle
./package.sh

# 3. Create DMG installer
./create_dmg.sh
```

### Quick repackage (no rebuild):
```bash
./package.sh && ./create_dmg.sh
```

## 🎨 Icon Resources

If you don't have an icon yet, here are some options:

1. **SF Symbols** (macOS built-in):
   - Use Mic or Caption related symbols
   - Open "SF Symbols" app on your Mac

2. **Free Icon Sources**:
   - https://www.flaticon.com (search "microphone" or "caption")
   - https://iconscout.com
   - https://www.iconfinder.com

3. **Design Your Own**:
   - Use Figma, Sketch, or Canva
   - Export as 1024x1024 PNG

## 📝 Notes

- The DMG uses UDZO compression (optimized for distribution)
- All libraries are bundled (no external dependencies needed)
- App is code-signed for basic macOS compatibility
- For App Store or Gatekeeper notarization, additional steps needed

## 🐛 Troubleshooting

### DMG not mounting?
```bash
rm -f LiveCaptions-1.0.dmg
./create_dmg.sh
```

### Icon not showing?
```bash
# Clear icon cache
sudo rm -rf /Library/Caches/com.apple.iconservices.store
killall Finder
```

### App not launching?
```bash
# Test directly
./LiveCaptions.app/Contents/MacOS/livecaptions
```
