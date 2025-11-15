#!/bin/bash

# Convert icon to macOS .icns format
# Usage: ./create_icon.sh <input_image.png>

set -e

if [ $# -eq 0 ]; then
    echo "Usage: ./create_icon.sh <input_image.png>"
    echo ""
    echo "The input image should ideally be 1024x1024 pixels"
    exit 1
fi

INPUT_IMAGE="$1"
ICONSET="LiveCaptions.iconset"
OUTPUT_ICNS="LiveCaptions.icns"

if [ ! -f "$INPUT_IMAGE" ]; then
    echo "❌ Error: $INPUT_IMAGE not found"
    exit 1
fi

echo "🎨 Creating icon from $INPUT_IMAGE..."

# Clean up old iconset
rm -rf "$ICONSET"
mkdir -p "$ICONSET"

# Generate all required sizes
sips -z 16 16     "$INPUT_IMAGE" --out "$ICONSET/icon_16x16.png"
sips -z 32 32     "$INPUT_IMAGE" --out "$ICONSET/icon_16x16@2x.png"
sips -z 32 32     "$INPUT_IMAGE" --out "$ICONSET/icon_32x32.png"
sips -z 64 64     "$INPUT_IMAGE" --out "$ICONSET/icon_32x32@2x.png"
sips -z 128 128   "$INPUT_IMAGE" --out "$ICONSET/icon_128x128.png"
sips -z 256 256   "$INPUT_IMAGE" --out "$ICONSET/icon_128x128@2x.png"
sips -z 256 256   "$INPUT_IMAGE" --out "$ICONSET/icon_256x256.png"
sips -z 512 512   "$INPUT_IMAGE" --out "$ICONSET/icon_256x256@2x.png"
sips -z 512 512   "$INPUT_IMAGE" --out "$ICONSET/icon_512x512.png"
sips -z 1024 1024 "$INPUT_IMAGE" --out "$ICONSET/icon_512x512@2x.png"

# Convert to icns
iconutil -c icns "$ICONSET" -o "$OUTPUT_ICNS"

# Clean up
rm -rf "$ICONSET"

echo "✅ Icon created: $OUTPUT_ICNS"
echo ""
echo "Next steps:"
echo "  1. Copy $OUTPUT_ICNS to LiveCaptions.app/Contents/Resources/"
echo "  2. Run ./package.sh to rebuild the app with the new icon"
