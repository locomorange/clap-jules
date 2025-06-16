#!/bin/bash

# Skia setup script for clap-jules project
# Based on https://skia.org/docs/user/download/ and https://skia.org/docs/user/build/

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SKIA_DIR="$PROJECT_ROOT/third_party/skia"

echo "Setting up Skia for clap-jules project..."

# Check if depot_tools is available
if ! command -v gn &> /dev/null; then
    echo "Error: depot_tools not found. Please install depot_tools first:"
    echo "  git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'"
    echo "  export PATH=\"\${PWD}/depot_tools:\${PATH}\""
    exit 1
fi

# Create third_party directory if it doesn't exist
mkdir -p "$PROJECT_ROOT/third_party"

# Clone Skia if not already present
if [ ! -d "$SKIA_DIR" ]; then
    echo "Cloning Skia repository..."
    cd "$PROJECT_ROOT/third_party"
    git clone https://skia.googlesource.com/skia.git
    cd skia
else
    echo "Skia directory already exists, updating..."
    cd "$SKIA_DIR"
    git pull
fi

# Sync dependencies
echo "Syncing Skia dependencies..."
python tools/git-sync-deps

# Configure build
echo "Configuring Skia build..."
# Use a minimal configuration suitable for our CLAP plugin
bin/gn gen out/Release --args='
    is_debug=false
    is_official_build=true
    skia_use_system_libjpeg_turbo=false
    skia_use_system_libpng=false
    skia_use_system_libwebp=false
    skia_use_system_zlib=false
    skia_use_sfntly=false
    skia_use_freetype=true
    skia_use_harfbuzz=true
    skia_use_icu=false
    skia_enable_tools=false
    skia_enable_skshaper=true
    skia_enable_svg=false
    skia_enable_pdf=false
    extra_cflags=["-fPIC"]
'

# Build Skia
echo "Building Skia (this may take a while)..."
ninja -C out/Release

echo "Skia setup completed successfully!"
echo "Skia libraries are available in: $SKIA_DIR/out/Release"