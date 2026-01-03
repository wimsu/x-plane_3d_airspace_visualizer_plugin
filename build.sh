#!/bin/bash

# Build script for X-Plane Airspace Visualizer Plugin
# For macOS x86_64 (works on Apple Silicon via Rosetta 2)

set -e

SDK_PATH="./SDK"
SOURCE_FILE="src/bayairspace_final.cpp"
OUTPUT_DIR="output"
OUTPUT="$OUTPUT_DIR/airspace_visualizer_mac.xpl"

# Check if SDK exists
if [ ! -d "$SDK_PATH" ]; then
    echo "Error: SDK not found at $SDK_PATH"
    echo "Please ensure the X-Plane SDK is in the SDK/ directory"
    exit 1
fi

# Check if source file exists
if [ ! -f "$SOURCE_FILE" ]; then
    echo "Error: Source file $SOURCE_FILE not found"
    exit 1
fi

# Create output directory if it doesn't exist
if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
    echo "Created output directory: $OUTPUT_DIR"
fi

echo "Building $OUTPUT for x86_64..."
echo "Using SDK: $SDK_PATH"
echo "Source: $SOURCE_FILE"

# Try to find json-c in common locations
JSONC_INCLUDE=""
JSONC_LIB=""

if [ -d "/usr/local/include/json-c" ]; then
    JSONC_INCLUDE="-I/usr/local/include/json-c"
    JSONC_LIB="-L/usr/local/lib"
elif [ -d "/opt/homebrew/include/json-c" ]; then
    JSONC_INCLUDE="-I/opt/homebrew/include/json-c"
    JSONC_LIB="-L/opt/homebrew/lib"
else
    echo "Warning: json-c not found in common locations"
    echo "Trying default paths..."
    JSONC_INCLUDE="-I/usr/local/include/json-c"
    JSONC_LIB="-L/usr/local/lib"
fi

clang++ -arch x86_64 -std=c++17 -fPIC -shared -o "$OUTPUT" \
    -I"$SDK_PATH/CHeaders" \
    -I"$SDK_PATH/CHeaders/XPLM" \
    $JSONC_INCLUDE \
    -F"$SDK_PATH/Libraries/Mac" \
    $JSONC_LIB \
    -ljson-c \
    -DAPL=1 \
    -DGL_SILENCE_DEPRECATION \
    -DXPLM303 \
    -framework XPLM \
    -framework XPWidgets \
    -framework OpenGL \
    "$SOURCE_FILE"

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Build complete: $OUTPUT"
    echo ""
    echo "Install to: X-Plane/Resources/plugins/airspace_visualizer_plugin/airspace_visualizer_mac.xpl"
else
    echo ""
    echo "✗ Build failed"
    exit 1
fi

