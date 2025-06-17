# clap-jules

A CLAP (CLever Audio Plugin) development project with integrated validation and analysis tools, featuring GUI support through GLFW.

## Features

- **Audio Processing**: Basic CLAP audio plugin functionality
- **GUI Support**: Cross-platform GUI implementation using GLFW
- **Cross-Platform**: Supports Windows (Win32), macOS (Cocoa), and Linux (X11)
- **Validation Tools**: Integrated CLAP validation and analysis tools

## Dependencies

### Core Dependencies
- **CMake** 3.16 or higher
- **C++17** compatible compiler
- **CLAP SDK** (included as submodule)

### GUI Dependencies
- **GLFW** (included as submodule)
- **OpenGL** (system dependency)

### Platform-Specific Dependencies

#### Linux
```bash
sudo apt-get install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

#### Windows
- Visual Studio with Windows SDK
- OpenGL drivers

#### macOS  
- Xcode command line tools
- OpenGL framework (included with macOS)

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

### Prerequisites

Make sure you have all the required dependencies installed for your platform (see Dependencies section above).

### Build Steps

```bash
# Initialize submodules (includes CLAP, GLFW, clap-helpers, and GoogleTest)
git submodule update --init --recursive

# Configure and build
cmake . -B build
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

The built plugin will be available as:
- **Linux**: `build/MyFirstClapPlugin.so`
- **Windows**: `build/Release/MyFirstClapPlugin.clap`  
- **macOS**: `build/MyFirstClapPlugin.dylib`

### GUI Features

The plugin now includes GUI support with the following capabilities:

- **Cross-platform window management** via GLFW
- **CLAP GUI extension** implementation
- **Embedded and floating window** support
- **Platform-specific rendering contexts**:
  - X11 embedding on Linux
  - Win32 embedding on Windows  
  - Cocoa embedding on macOS
- **Resizable interface** with proper aspect ratio handling

### Future Skia Integration

The project is prepared for Skia graphics library integration for advanced 2D rendering capabilities. The current implementation provides a foundation that can be extended with Skia-based rendering.

## Architecture

### Plugin Structure

The plugin implements the following CLAP extensions:

- **Core Plugin API**: Basic audio processing and lifecycle management
- **GUI Extension**: Cross-platform graphical user interface
- **Future Extensions**: Audio ports, parameters, state management

### GUI Implementation

- **GLFW Backend**: Cross-platform window and context management
- **OpenGL Rendering**: Hardware-accelerated graphics context
- **Platform Abstraction**: Unified interface for Win32, X11, and Cocoa
- **Event Handling**: Mouse, keyboard, and window events