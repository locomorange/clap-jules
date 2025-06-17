# clap-jules

A CLAP (CLever Audio Plugin) development project with integrated validation, analysis tools, and GUI support.

## Features

- **Complete CLAP Plugin Implementation**: Basic audio effect with GUI support
- **Cross-Platform GUI**: GLFW-based windowing with OpenGL rendering
- **Graphics Rendering**: Simple 2D graphics system with extensible architecture
- **Platform Support**: Windows (Win32), Linux (X11), and macOS (Cocoa) window embedding
- **Development Tools**: Integrated CLAP validator and info tools

## Dependencies

- **CMake** 3.16 or higher
- **C++17** compatible compiler
- **GLFW3** (for windowing and input)
- **OpenGL** (for 2D graphics rendering)
- **CLAP SDK** (included as submodule)

## Building

```bash
# Install system dependencies (Ubuntu/Debian)
sudo apt update
sudo apt install -y libglfw3-dev libgl1-mesa-dev

# Initialize submodules
git submodule update --init --recursive

# Configure and build
cmake . -B build
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

The built plugin will be available as `build/MyFirstClapPlugin.so` on Linux or `build/Release/MyFirstClapPlugin.clap` on Windows.

## GUI Features

The plugin includes a working GUI system with:

- **CLAP GUI Extension**: Full implementation of clap.gui extension
- **2D Graphics**: Basic OpenGL-based rendering system
- **Cross-Platform**: Support for X11 (Linux), Win32 (Windows), and Cocoa (macOS)
- **Resizable Interface**: Dynamic window sizing and viewport management
- **Visual Elements**: Title bar, buttons, status indicators, and decorative graphics

### Current Graphics Implementation

The current implementation uses a simple OpenGL-based 2D renderer with:
- Rectangle drawing primitives
- Basic text rendering (placeholder system)
- Color support with alpha blending
- Viewport management and scaling

### Upgrading to Skia Graphics

For more advanced 2D graphics capabilities, the system is designed to accommodate Google's Skia library:

1. **Architecture**: The `simple_renderer_t` can be replaced with a Skia-based renderer
2. **Integration Options**:
   - **vcpkg**: `vcpkg install skia[gl]` (requires complex build dependencies)
   - **Prebuilt Libraries**: Download Skia prebuilt binaries for your platform
   - **Source Build**: Follow [Skia build instructions](https://skia.org/docs/user/build/)

3. **Implementation Path**:
   ```cpp
   // Replace simple_renderer_t with SkiaRenderer
   // Update renderer.h and renderer.cpp to use Skia APIs
   // Maintain the same interface for GUI integration
   ```

The graphics abstraction layer in `renderer.h` is designed to make this transition straightforward.

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