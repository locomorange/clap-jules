# clap-jules

A CLAP (CLever Audio Plugin) development project with integrated validation and analysis tools.

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

### Basic Build

```bash
# Initialize submodules
git submodule update --init --recursive

# Configure and build
cmake . -B build
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

The built plugin will be available as `build/MyFirstClapPlugin.so` on Linux or `build/Release/MyFirstClapPlugin.clap` on Windows.

### Graphics Library Support

This project includes optional support for graphics libraries:

#### GLFW Integration

GLFW is automatically included as a submodule and built with the project. It provides cross-platform windowing and input handling capabilities.

- **Status**: ✅ Fully integrated
- **Usage**: Automatically linked when building the plugin
- **Headers**: Available via `#include <GLFW/glfw3.h>`
- **Compile Definition**: `HAVE_GLFW=1` is defined when GLFW is available

#### Skia Integration

Skia is a complete 2D graphics library for drawing text, geometries, and images.

- **Status**: 🚧 Infrastructure ready, requires manual build
- **Submodule**: Available in `libs/skia`
- **Build Required**: Skia must be built separately before use

**To enable Skia support:**

1. **Install Dependencies** (Ubuntu/Debian):
   ```bash
   sudo apt-get update
   sudo apt-get install -y python3 build-essential git
   ```

2. **Install Depot Tools** (required by Skia):
   ```bash
   git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
   export PATH="${PWD}/depot_tools:${PATH}"
   ```

3. **Build Skia**:
   ```bash
   cd libs/skia
   python3 tools/git-sync-deps
   bin/gn gen out/Release --args='is_debug=false'
   ninja -C out/Release
   cd ../..
   ```

4. **Build with Skia enabled**:
   ```bash
   cmake . -B build -DUSE_SKIA=ON
   cmake --build build --config Release
   ```

**Alternative: Using Pre-built Skia**

If you have pre-built Skia libraries, you can specify their location:

```bash
cmake . -B build -DUSE_SKIA=ON \
  -DSKIA_INCLUDE_DIR=/path/to/skia/include \
  -DSKIA_LIBRARY=/path/to/skia/out/Release/libskia.a
```

When Skia is enabled:
- **Headers**: Available via `#include <skia/core/SkCanvas.h>`
- **Compile Definition**: `HAVE_SKIA=1` is defined
- **Linking**: Automatically linked to the plugin

### System Requirements

For graphics support, ensure you have the appropriate development libraries:

**Linux:**
```bash
# For X11 support (if needed)
sudo apt-get install -y libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

# For Wayland support (if needed)
sudo apt-get install -y libwayland-dev wayland-protocols libxkbcommon-dev
```

**Note**: The current configuration builds GLFW with minimal dependencies (null backend) to ensure broad compatibility. For full windowing system support, enable the appropriate backends in CMakeLists.txt.