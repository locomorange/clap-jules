# clap-jules

A CLAP (CLever Audio Plugin) development project with integrated validation, analysis tools, and graphics rendering capabilities using Skia.

## Features

- **CLAP Plugin Framework**: Complete CLAP audio plugin implementation
- **Graphics Rendering**: Integrated Skia graphics library support for plugin GUI rendering
- **Development Tools**: Automated validation and analysis tools
- **Testing**: Comprehensive test suite with Google Test

## Graphics Integration (Skia)

This project includes integrated support for the Skia 2D graphics library, enabling:

- **Custom Plugin GUIs**: Create rich, hardware-accelerated plugin interfaces
- **Visual Effects**: Real-time spectrum analysis, waveform displays, and custom visualizations  
- **Cross-Platform Rendering**: Consistent graphics rendering across different platforms
- **Performance**: Hardware-accelerated 2D graphics operations

### Skia Setup

The Skia graphics library is included as a git submodule but requires compilation:

```bash
# Initialize Skia submodule (if not already done)
git submodule update --init --recursive

# Build Skia (requires Python and depot_tools)
cd libs/skia
python tools/git-sync-deps
bin/gn gen out/Release --args='is_debug=false'
ninja -C out/Release
cd ../..

# Build plugin with Skia support
cmake . -B build -DENABLE_SKIA=ON
cmake --build build --config Release
```

### Graphics API Usage

The plugin includes a graphics renderer interface that automatically uses Skia when available:

```cpp
#include "graphics_renderer.h"

// Create renderer (uses Skia if available, falls back to basic implementation)
auto renderer = clap_jules::createGraphicsRenderer();

// Basic drawing operations
renderer->beginFrame(width, height);
renderer->clear(0xFF000000);
renderer->drawRect(x, y, width, height, color);
renderer->drawCircle(x, y, radius, color);
renderer->drawText("Hello", x, y, color);
renderer->endFrame();
```

The graphics system demonstrates:
- Automatic Skia integration when available
- Fallback to basic software rendering
- Plugin GUI framework integration
- Example spectrum visualization

## Development Tools

This project includes automated setup for essential CLAP development tools in the GitHub Actions workflow:

### CLAP Validator
- **Version**: 0.3.2
- **Purpose**: Comprehensive plugin validation and testing
- **Usage**: Automatically runs `clap-validator validate` on built plugins
- **Manual Usage**: `clap-validator validate path/to/plugin.so`

### CLAP Info
- **Version**: 1.2.2  
- **Purpose**: Extract detailed plugin information and capabilities
- **Usage**: Automatically runs `clap-info --brief` on built plugins
- **Manual Usage**: `clap-info path/to/plugin.so [options]`

Both tools are automatically downloaded and configured in the GitHub Actions workflow for Ubuntu/Linux builds. They provide validation feedback and detailed plugin analysis to ensure CLAP compatibility and quality.

## Building

### Basic Build (without Skia)

```bash
# Initialize submodules
git submodule update --init --recursive

# Configure and build
cmake . -B build
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

### Build with Skia Graphics Support

```bash
# Initialize submodules including Skia
git submodule update --init --recursive

# Build Skia (one-time setup)
cd libs/skia
python tools/git-sync-deps
bin/gn gen out/Release --args='is_debug=false'
ninja -C out/Release
cd ../..

# Build plugin with Skia support
cmake . -B build -DENABLE_SKIA=ON
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

### Build Outputs

- **Basic build**: `build/MyFirstClapPlugin.so` (Linux) or `build/Release/MyFirstClapPlugin.clap` (Windows)
- **With Skia**: Includes graphics rendering capabilities for custom plugin GUIs

### Graphics Features Demonstration

When the plugin is activated, it automatically demonstrates graphics rendering capabilities:
- Background clearing and color fills
- Rectangle and circle drawing primitives  
- Text rendering (with Skia) or placeholder rendering (basic mode)
- Spectrum visualization bars example