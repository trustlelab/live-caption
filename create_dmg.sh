#!/bin/bash

# Create DMG for Live Captions

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_NAME="Live Captions"
APP_BUNDLE="$SCRIPT_DIR/LiveCaptions.app"
DMG_NAME="LiveCaptions-1.0.dmg"
VOLUME_NAME="Live Captions Installer"
DMG_TEMP="$SCRIPT_DIR/dmg_temp"
ICON_FILE="$SCRIPT_DIR/LiveCaptions.icns"

echo "📦 Creating DMG installer for $APP_NAME..."

# Check if app bundle exists
if [ ! -d "$APP_BUNDLE" ]; then
    echo "❌ Error: $APP_BUNDLE not found. Please run ./package.sh first."
    exit 1
fi

# Clean up previous DMG
rm -f "$SCRIPT_DIR/$DMG_NAME"
rm -rf "$DMG_TEMP"

# Create temporary directory
mkdir -p "$DMG_TEMP"

# Copy app to temp directory
echo "Copying application..."
cp -R "$APP_BUNDLE" "$DMG_TEMP/"

# Copy setup guide if it exists
if [ -f "$SCRIPT_DIR/SETUP_MACOS.txt" ]; then
    echo "Including setup guide..."
    cp "$SCRIPT_DIR/SETUP_MACOS.txt" "$DMG_TEMP/"
fi

# Create Applications symlink
echo "Creating Applications symlink..."
ln -s /Applications "$DMG_TEMP/Applications"

# Create DMG
echo "Creating DMG..."
hdiutil create -volname "$VOLUME_NAME" \
    -srcfolder "$DMG_TEMP" \
    -ov -format UDZO \
    "$SCRIPT_DIR/$DMG_NAME"

# Set custom icon on DMG if available
if [ -f "$ICON_FILE" ]; then
    echo "Setting custom DMG icon..."
    
    # Convert to read-write format
    hdiutil convert "$SCRIPT_DIR/$DMG_NAME" -format UDRW -o "$SCRIPT_DIR/${DMG_NAME}.temp"
    
    # Mount the DMG
    MOUNT_POINT=$(hdiutil attach "$SCRIPT_DIR/${DMG_NAME}.temp.dmg" | grep "/Volumes" | tail -1 | sed 's/.*\(\/Volumes\/.*\)/\1/')
    
    if [ -n "$MOUNT_POINT" ]; then
        echo "Mounted at: $MOUNT_POINT"
        # Copy icon file to volume's hidden icon
        cp "$ICON_FILE" "$MOUNT_POINT/.VolumeIcon.icns"
        
        # Set the custom icon attribute
        SetFile -a C "$MOUNT_POINT" 2>/dev/null || true
        
        # Unmount
        hdiutil detach "$MOUNT_POINT"
    fi
    
    # Convert back to compressed format
    rm -f "$SCRIPT_DIR/$DMG_NAME"
    hdiutil convert "$SCRIPT_DIR/${DMG_NAME}.temp.dmg" -format UDZO -o "$SCRIPT_DIR/$DMG_NAME"
    rm -f "$SCRIPT_DIR/${DMG_NAME}.temp.dmg"
fi

# Clean up
rm -rf "$DMG_TEMP"

echo "✅ DMG created successfully!"
echo "📍 Location: $SCRIPT_DIR/$DMG_NAME"
echo ""
echo "To distribute:"
echo "  1. Double-click $DMG_NAME to mount it"
echo "  2. Drag 'Live Captions' to the Applications folder"
