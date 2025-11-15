#!/bin/bash

# Create a simple placeholder icon using macOS tools

set -e

TEMP_PNG="temp_icon.png"
OUTPUT_ICNS="LiveCaptions.icns"

echo "🎨 Creating placeholder icon..."

# Create a simple icon using SF Symbols or create a basic one
# For now, let's create a simple colored square with text
sips -s format png --out "$TEMP_PNG" -z 1024 1024 -c 1024 1024 -s formatOptions best /System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/GenericApplicationIcon.icns 2>/dev/null || {
    echo "Creating basic placeholder..."
    # If the above fails, we'll just note that user needs to provide an icon
}

if [ -f "$TEMP_PNG" ]; then
    ./create_icon.sh "$TEMP_PNG"
    rm "$TEMP_PNG"
else
    echo "⚠️  Could not create automatic icon."
    echo ""
    echo "Please provide your own icon image (PNG, 1024x1024 recommended)"
    echo "Then run: ./create_icon.sh your_icon.png"
fi
