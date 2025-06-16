# Skia Integration Guide

This document explains how to integrate Google's Skia 2D graphics library with the CLAP-Jules plugin.

## Overview

The project includes a graphics abstraction layer that can use either:
- **Skia 2D Graphics Library** - Full-featured graphics rendering (when available)
- **Stub Graphics Implementation** - Fallback with basic software rendering

## Current Status

✅ Graphics abstraction layer implemented  
✅ Stub graphics backend functional  
✅ CMake configuration for Skia detection  
✅ Plugin demonstrates graphics usage  
✅ Tests validate graphics functionality  
⚠️ Skia integration requires manual setup (see below)

## Quick Start

The project builds and works without Skia installed, using a stub graphics implementation:

```bash
# Build with stub graphics (default)
cmake . -B build
cmake --build build --config Release
```

To see the plugin in action:
```bash
./clap-info --brief build/MyFirstClapPlugin.so
```

## Enabling Skia Support

### Option 1: Install Skia System-wide (Ubuntu/Debian)

```bash
# Install Skia development packages (if available in your distribution)
sudo apt-get install libskia-dev

# Build with Skia enabled
cmake . -B build -DCLAP_JULES_USE_SKIA=ON
cmake --build build --config Release
```

### Option 2: Manual Skia Installation

1. **Download and build Skia:**

```bash
# Create third-party directory
mkdir -p third_party && cd third_party

# Clone Skia
git clone https://skia.googlesource.com/skia.git
cd skia

# Install depot_tools (if not already installed)
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH="${PWD}/../depot_tools:${PATH}"

# Sync dependencies
python tools/git-sync-deps

# Build Skia
bin/gn gen out/Release --args='is_official_build=true'
ninja -C out/Release
```

2. **Configure CLAP-Jules to use your Skia build:**

```bash
# Set Skia paths and build
cmake . -B build \
  -DCLAP_JULES_USE_SKIA=ON \
  -DSKIA_INCLUDE_DIR=/path/to/skia \
  -DSKIA_LIBRARY=/path/to/skia/out/Release/libskia.a

cmake --build build --config Release
```

### Option 3: Using pkg-config

If Skia is installed with pkg-config support:

```bash
# Build with automatic Skia detection
cmake . -B build -DCLAP_JULES_USE_SKIA=ON
cmake --build build --config Release
```

## Verifying Skia Integration

When Skia is properly integrated, you should see:

```bash
./clap-info --brief build/MyFirstClapPlugin.so
# Output should show: "Graphics backend - Skia 2D Graphics Library"
# Instead of: "Graphics backend - Stub Graphics Implementation"
```

## Graphics API Usage

The graphics API is designed to be simple and familiar:

```cpp
#include "graphics/skia_graphics.h"

// Create graphics context
auto graphics = clap_jules::graphics::createGraphicsContext(320, 240);

// Draw basic shapes
graphics->clear(clap_jules::graphics::Color(50, 50, 50));
graphics->drawRect(clap_jules::graphics::Rect(10, 10, 100, 50), 
                  clap_jules::graphics::Color(255, 100, 100));
graphics->drawCircle(clap_jules::graphics::Point(160, 120), 30, 
                   clap_jules::graphics::Color(100, 255, 100));
graphics->drawText("Hello World", clap_jules::graphics::Point(50, 200),
                  clap_jules::graphics::Color(255, 255, 255), 16.0f);
```

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CLAP_JULES_USE_SKIA` | `OFF` | Enable Skia 2D graphics support |
| `CLAP_JULES_BUILD_GRAPHICS_EXAMPLES` | `ON` | Build graphics examples |
| `SKIA_INCLUDE_DIR` | (auto-detect) | Manual path to Skia headers |
| `SKIA_LIBRARY` | (auto-detect) | Manual path to Skia library |

## Architecture

The graphics system uses an abstract interface (`GraphicsContext`) with multiple backends:

- **SkiaGraphicsContext**: Full Skia implementation (when available)
- **StubGraphicsContext**: Software fallback implementation

This design allows the plugin to work with or without Skia, while providing a consistent API for drawing operations.

## Future Enhancements

- [ ] Pre-built Skia binaries for common platforms
- [ ] Additional graphics backends (Cairo, Direct2D, etc.)
- [ ] Advanced graphics features (gradients, paths, filters)
- [ ] GUI framework integration
- [ ] Performance optimizations

## Troubleshooting

**Issue**: CMake can't find Skia  
**Solution**: Use manual paths with `-DSKIA_INCLUDE_DIR` and `-DSKIA_LIBRARY`

**Issue**: Linking errors with Skia  
**Solution**: Ensure all Skia dependencies are available (freetype, harfbuzz, etc.)

**Issue**: Plugin shows stub graphics instead of Skia  
**Solution**: Verify that `CLAP_JULES_USE_SKIA=ON` was set during CMake configuration

For more help, check the CMake output for Skia detection messages.