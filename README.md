# clap-jules

A CLAP (CLever Audio Plugin) development project with integrated validation, analysis tools, and GUI support featuring GLFW windowing and Skia graphics integration.

## Features

### Audio Plugin Capabilities
- Full CLAP (CLever Audio Plugin) specification compliance
- Audio processing with configurable input/output routing
- Parameter management and automation support
- Host integration with proper lifecycle management

### GUI Support
- **CLAP GUI Extension**: Complete implementation supporting embedded and floating windows
- **Cross-platform windowing**: GLFW-based window management
- **Platform-specific embedding**:
  - X11 embedding for Linux hosts using XReparentWindow
  - Win32 embedding for Windows hosts using SetParent
- **Real-time rendering**: OpenGL-based graphics with basic visual feedback
- **Skia graphics integration**: Ready for advanced 2D graphics rendering (chrome/m138)

### Graphics Libraries
- **GLFW 3.4**: Cross-platform windowing and input handling
- **Skia (chrome/m138)**: Advanced 2D graphics library (headers available, build integration in progress)
- **OpenGL**: Hardware-accelerated rendering backend

## Dependencies

The project uses the following libraries as git submodules:

- **CLAP**: Core audio plugin framework
- **GLFW 3.4**: Windowing and input handling
- **Skia chrome/m138**: 2D graphics rendering library
- **clap-helpers**: Utility functions for CLAP development
- **GoogleTest**: Unit testing framework

## System Requirements

### Linux
```bash
# Install required development packages
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libx11-dev \
    libxcursor-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxi-dev \
    libxext-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev
```

### Windows
- Visual Studio 2019 or later with C++ support
- CMake 3.16 or later
- Git for Windows

## Building

```bash
# Clone with submodules
git clone --recursive https://github.com/locomorange/clap-jules.git
cd clap-jules

# Or if already cloned, initialize submodules
git submodule update --init --recursive

# Configure with CMake
cmake . -B build

# Build the project
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

## Plugin Output

The built plugin will be available as:
- **Linux**: `build/MyFirstClapPlugin.so`
- **Windows**: `build/Release/MyFirstClapPlugin.clap`

## GUI Capabilities

The plugin includes a fully functional GUI with:

1. **Window Management**: Creates resizable windows with proper host integration
2. **Visual Feedback**: Real-time OpenGL rendering with colorful gradients and basic shapes
3. **Host Embedding**: Seamless integration into DAW plugin windows
4. **Cross-platform Support**: Works on Linux (X11) and Windows (Win32)

### Testing the GUI

Most CLAP-compatible DAWs will automatically detect and display the plugin's GUI when loaded. The plugin provides:
- Colorful gradient background rendering
- Interactive window resizing
- Proper window embedding in host applications

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

### CLAP Host Integration Testing
- **Purpose**: Integration testing with reference CLAP host and GUI screenshot capture
- **Repository**: https://github.com/free-audio/clap-host
- **Integration**: Runs automatically after plugin build in GitHub Actions
- **Platforms**: Windows, macOS, Linux with platform-specific screenshot methods
- **Artifacts**: Screenshots and diagnostic logs are uploaded as workflow artifacts
- **Caching**: CLAP host builds are cached for faster subsequent runs

The integrated workflow automatically:
1. Builds the plugin first, then runs CLAP host testing in parallel with other tests
2. Downloads plugin artifacts from the build stage
3. Uses cached CLAP host builds when available to reduce build time
4. Installs proper Qt6 dependencies for successful builds
5. Loads the built plugin into the host application
6. Captures screenshots showing the plugin interface with window decorations
7. Uploads screenshots and diagnostic logs as artifacts for inspection

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

## Future Enhancements

### Skia Integration
The project includes Skia (chrome/m138) as a submodule for advanced 2D graphics capabilities. Skia integration is currently in preparation phase:

- ✅ Skia source code available as git submodule
- ✅ Headers detected and ready for integration
- 🔄 CMake build integration (due to Skia's GN/Ninja build system complexity)
- 📋 Planned features: Advanced vector graphics, text rendering, image processing

### Build Integration Options
For full Skia integration, several approaches are being considered:
1. **GN-to-CMake conversion**: Using Skia's `gn_to_cmake.py` script
2. **Precompiled libraries**: Using pre-built Skia binaries
3. **Minimal subset**: Custom CMake build for essential Skia components

Current implementation provides a solid foundation with OpenGL rendering that can be enhanced with Skia's advanced capabilities.