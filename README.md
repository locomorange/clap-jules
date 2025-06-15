# clap-jules

A CLAP (CLever Audio Plugin) development project with integrated validation and analysis tools, featuring a professional soothe2-style GUI interface.

## Professional Audio Plugin GUI

This project implements a high-quality audio plugin GUI similar to soothe2, featuring:

### Main Interface Features
- **Professional soothe2-style layout** with left-side controls and right-side EQ visualization
- **Interactive EQ curve editing** with drag-and-drop point manipulation
- **Comprehensive parameter automation** through CLAP parameter bindings
- **Dark professional theme** optimized for studio environments
- **Real-time visual feedback** for all control changes

### Left Side Control Panel
- **DEPTH** - Large main control knob for primary effect intensity
- **SHARPNESS** - Controls the precision of the dynamic EQ response  
- **SELECTIVITY** - Adjusts frequency selectivity of the processing
- **ATTACK/RELEASE** - Envelope controls for dynamic response timing
- **MODE** - Radio button selection (Soft/Medium/Hard processing modes)
- **STEREO LINK** - Toggle for linked stereo processing
- **QUALITY** - High-quality processing mode switch
- **BALANCE** - Left/Right balance control slider

### Right Side EQ Visualization
- **Interactive EQ curve display** with real-time frequency response
- **Draggable EQ points** - Double-click to add, right-click to remove
- **Undo/Redo functionality** for EQ curve editing
- **Reset button** to restore default EQ settings
- **Professional grid overlay** with frequency and gain markings

### GUI Usage Examples

![image1](assets/images/image1.md)

#### Basic Operation:
1. **Main Controls**: Adjust DEPTH for primary effect intensity
2. **Shaping**: Use SHARPNESS and SELECTIVITY to fine-tune response
3. **Dynamics**: Set ATTACK and RELEASE for envelope behavior
4. **Mode Selection**: Choose processing mode via MODE buttons
5. **EQ Editing**: 
   - Double-click in EQ area to add frequency points
   - Drag existing points to adjust frequency and gain
   - Right-click points to remove them
   - Use UNDO/REDO buttons for operation history

#### Advanced Features:
- **Parameter Automation**: All controls support host automation
- **Stereo Processing**: Toggle STEREO LINK for independent L/R control  
- **Quality Mode**: Enable for higher-precision processing
- **Visual Feedback**: Real-time EQ curve updates with parameter changes

## Features

- **CLAP Plugin Framework**: Modern audio plugin architecture
- **VSTGUI Integration**: Graphical user interface support using VSTGUI 4.14.3
- **Professional GUI Design**: Soothe2-style interface with comprehensive controls
- **Parameter Automation**: Full CLAP parameter extension support
- **Interactive EQ Editing**: Real-time curve manipulation with undo/redo
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