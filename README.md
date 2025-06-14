# clap-jules

A CLAP (CLever Audio Plugin) development project with integrated validation and analysis tools and VSTGUI support for graphical user interfaces.

## Features

- **CLAP Plugin Framework**: Modern audio plugin architecture
- **VSTGUI Integration**: Graphical user interface support using VSTGUI 4.14.3
- **Automated Validation**: Built-in CLAP validator for plugin testing
- **Cross-platform Support**: Linux, macOS, and Windows compatibility

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
# Initialize submodules (includes CLAP, clap-helpers, googletest, and VSTGUI)
git submodule update --init --recursive

# Install system dependencies for VSTGUI on Linux
sudo apt install -y libx11-dev libxcursor-dev libxrandr-dev libgl1-mesa-dev \
                    libasound2-dev libfreetype6-dev libxcb-util-dev \
                    libxcb-keysyms1-dev libxkbcommon-dev libxkbcommon-x11-dev \
                    libglib2.0-dev libcairo2-dev libpango1.0-dev libgtkmm-3.0-dev

# Configure and build
cmake . -B build
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

The built plugin will be available as `build/MyFirstClapPlugin.so` on Linux or `build/Release/MyFirstClapPlugin.clap` on Windows.

### Dependencies

- **CMake 3.25+**: Required for VSTGUI support
- **C++17 compiler**: Required by both CLAP and VSTGUI
- **System GUI libraries**: X11, Cairo, Pango, GTK (Linux); Cocoa (macOS); Win32 (Windows)