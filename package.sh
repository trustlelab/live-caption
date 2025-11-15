#!/bin/bash

# Package Live Captions into a macOS .app bundle

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_BUNDLE="$SCRIPT_DIR/LiveCaptions.app"
BUILD_DIR="$SCRIPT_DIR/builddir"

echo "📦 Packaging Live Captions..."

# Check if build exists
if [ ! -f "$BUILD_DIR/src/livecaptions" ]; then
    echo "❌ Error: Build not found. Please run 'ninja -C builddir' first."
    exit 1
fi

# Create app bundle structure
echo "Creating app bundle structure..."
mkdir -p "$APP_BUNDLE/Contents/MacOS"
mkdir -p "$APP_BUNDLE/Contents/Resources"
mkdir -p "$APP_BUNDLE/Contents/Frameworks"

# Create Info.plist
cat > "$APP_BUNDLE/Contents/Info.plist" << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>livecaptions</string>
    <key>CFBundleIdentifier</key>
    <string>net.sapples.LiveCaptions</string>
    <key>CFBundleName</key>
    <string>Live Captions</string>
    <key>CFBundleDisplayName</key>
    <string>Live Captions</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>????</string>
    <key>CFBundleIconFile</key>
    <string>LiveCaptions.icns</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.15</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSMicrophoneUsageDescription</key>
    <string>Live Captions needs access to your microphone to transcribe audio.</string>
</dict>
</plist>
EOF

# Copy the main executable
echo "Copying executable..."
cp "$BUILD_DIR/src/livecaptions" "$APP_BUNDLE/Contents/MacOS/"

# Remove all existing rpaths to prevent duplicates
echo "Cleaning rpaths from executable..."
while otool -l "$APP_BUNDLE/Contents/MacOS/livecaptions" | grep -q "LC_RPATH"; do
    RPATH=$(otool -l "$APP_BUNDLE/Contents/MacOS/livecaptions" | grep -A2 LC_RPATH | grep path | head -1 | awk '{print $2}')
    install_name_tool -delete_rpath "$RPATH" "$APP_BUNDLE/Contents/MacOS/livecaptions" 2>/dev/null || break
done

# Copy GSettings schema
echo "Copying GSettings schema..."
mkdir -p "$APP_BUNDLE/Contents/Resources/share/glib-2.0/schemas"
cp "$BUILD_DIR/data/gschemas.compiled" "$APP_BUNDLE/Contents/Resources/share/glib-2.0/schemas/"

# Copy icon files
echo "Copying icons..."
if [ -d "$BUILD_DIR/data/icons" ]; then
    mkdir -p "$APP_BUNDLE/Contents/Resources/share/icons"
    cp -r "$BUILD_DIR/data/icons/hicolor" "$APP_BUNDLE/Contents/Resources/share/icons/" 2>/dev/null || true
fi

# Copy .desktop and .appdata.xml files
echo "Copying metadata files..."
mkdir -p "$APP_BUNDLE/Contents/Resources/share/applications"
mkdir -p "$APP_BUNDLE/Contents/Resources/share/metainfo"
cp "$BUILD_DIR/data/net.sapples.LiveCaptions.desktop" "$APP_BUNDLE/Contents/Resources/share/applications/" 2>/dev/null || true
cp "$BUILD_DIR/data/net.sapples.LiveCaptions.appdata.xml" "$APP_BUNDLE/Contents/Resources/share/metainfo/" 2>/dev/null || true

# Copy april model
echo "Copying ASR model..."
cp "$SCRIPT_DIR/april-english-dev-01110_en.april" "$APP_BUNDLE/Contents/Resources/"

# Copy icon if it exists
if [ -f "$SCRIPT_DIR/LiveCaptions.icns" ]; then
    echo "Copying app icon..."
    cp "$SCRIPT_DIR/LiveCaptions.icns" "$APP_BUNDLE/Contents/Resources/"
fi

# Copy libraries using dylibbundler if available, otherwise provide instructions
if command -v dylibbundler &> /dev/null; then
    echo "Bundling dynamic libraries..."
    dylibbundler -od -b \
        -x "$APP_BUNDLE/Contents/MacOS/livecaptions" \
        -d "$APP_BUNDLE/Contents/Frameworks" \
        -p @executable_path/../Frameworks/
else
    echo "⚠️  dylibbundler not found. You'll need to manually bundle dependencies or install it:"
    echo "    brew install dylibbundler"
    echo ""
    echo "For now, the app will rely on system libraries (Homebrew paths)."
fi

# Make executable
chmod +x "$APP_BUNDLE/Contents/MacOS/livecaptions"

echo "✅ Packaging complete!"
echo "📍 Bundle location: $APP_BUNDLE"
echo ""
echo "To run: open $APP_BUNDLE"
