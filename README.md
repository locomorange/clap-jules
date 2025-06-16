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

### Building with Skia Graphics Support

This project supports optional Skia graphics integration for advanced 2D rendering capabilities.

#### Option 1: Using Prebuilt Skia Binaries (Recommended)

1. **Download Skia**:
   - Visit [Skia's official releases](https://github.com/google/skia/releases) or build from source following [Skia's build instructions](https://skia.org/docs/user/build/)
   - Extract the Skia distribution to a directory (e.g., `/opt/skia` or `C:\skia`)

2. **Build with Skia enabled**:
```bash
# Set the SKIA_PATH environment variable
export SKIA_PATH=/path/to/skia

# Or use CMake option directly
cmake . -B build -DENABLE_SKIA=ON -DSKIA_PATH=/path/to/skia
cmake --build build --config Release
```

#### Option 2: Building Skia from Source

For building Skia from source, refer to the [official Skia documentation](https://skia.org/docs/user/download/):

1. **Install Prerequisites**:
   ```bash
   # Install depot_tools
   git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
   export PATH="${PWD}/depot_tools:${PATH}"
   
   # Install Python 3 and other dependencies
   # On Ubuntu/Debian:
   sudo apt-get install python3 python3-pip
   ```

2. **Download and Build Skia**:
   ```bash
   # Clone Skia
   git clone https://skia.googlesource.com/skia.git
   cd skia
   
   # Sync dependencies
   python3 tools/git-sync-deps
   
   # Configure and build
   bin/gn gen out/Release --args='is_official_build=true'
   ninja -C out/Release
   ```

3. **Build clap-jules with Skia**:
   ```bash
   cd /path/to/clap-jules
   cmake . -B build -DENABLE_SKIA=ON -DSKIA_PATH=/path/to/skia
   cmake --build build --config Release
   ```

#### Skia Features

When Skia is enabled, the plugin gains access to advanced 2D graphics capabilities:

- **Hardware-accelerated rendering** (when supported)
- **Vector graphics and text rendering**
- **Image processing and effects**
- **Cross-platform graphics compatibility**

The project includes a graphics wrapper (`graphics_wrapper.h`) that provides a unified interface for 2D graphics operations. When Skia is not available, it falls back to a software renderer.

#### Verification

To verify Skia integration:

```bash
# Check if Skia was detected during configuration
cmake . -B build -DENABLE_SKIA=ON
# Look for "Skia integration enabled" in the output

# Run tests to verify graphics functionality
ctest --test-dir build --output-on-failure

# Build and run the graphics example (optional)
cmake . -B build -DBUILD_SKIA_EXAMPLE=ON
cmake --build build
./build/skia_example
```

#### Known Issues and Limitations

1. **Skia Complexity**: Building Skia from source is complex and requires significant build time and dependencies. Using prebuilt binaries is recommended for most users.

2. **Platform Dependencies**: Skia may require additional system libraries on some platforms:
   - **Linux**: May require OpenGL, X11, or Wayland development libraries
   - **Windows**: May require DirectX or ANGLE libraries
   - **macOS**: May require Metal or OpenGL frameworks

3. **Fallback Behavior**: When Skia is not available, the graphics wrapper falls back to a simple software renderer with basic functionality. This ensures the plugin remains functional even without Skia.

4. **Memory Usage**: Skia can be memory-intensive for large graphics operations. Consider the target environment when enabling Skia support.

5. **License Compatibility**: Ensure Skia's BSD license is compatible with your project's licensing requirements.

## GUI Support

This CLAP plugin includes a full GUI implementation using the CLAP GUI extension. The GUI displays a visual representation of the plugin's graphics capabilities.

### Features

- **Cross-platform windowing**: Supports Win32 (Windows), X11 (Linux), and Cocoa (macOS)
- **Embedded and floating window modes**: Can integrate into host windows or run as standalone windows
- **Resizable interface**: Supports dynamic resizing with constraints
- **Real-time graphics rendering**: Uses the integrated graphics wrapper for visual feedback
- **Host integration**: Fully compliant with CLAP GUI extension specifications

### GUI Architecture

The GUI implementation includes:

- **Platform abstraction**: Cross-platform window management code
- **Graphics integration**: Leverages the existing graphics wrapper for rendering
- **Host communication**: Proper CLAP extension protocol implementation
- **Resource management**: Automatic cleanup and memory management

### Visual Elements

The plugin GUI displays:
- Dark-themed interface with professional appearance
- Animated graphics showing plugin capabilities
- Real-time visual feedback
- Resizable graphics that adapt to window size changes

### Testing the GUI

#### Automated Testing
```bash
# Build with GUI test enabled (default)
cmake . -B build
cmake --build build

# Run GUI functionality test
./build/test_gui
```

#### Manual Testing with Host Applications
The plugin can be loaded into any CLAP-compatible host that supports the GUI extension:

1. **Load the plugin** in your CLAP host
2. **Open the plugin GUI** - look for a GUI/Editor button
3. **Resize the window** to test responsive graphics
4. **Observe real-time rendering** of graphics elements

#### Host Compatibility
The GUI has been designed to work with popular DAW hosts that support CLAP:
- Bitwig Studio (with CLAP support)
- Reaper (with CLAP plugin)
- Other CLAP-compatible hosts

### Platform-Specific Notes

#### Linux (X11)
- Requires X11 development libraries: `sudo apt-get install libx11-dev`
- Tested with common Linux desktop environments
- Supports both embedded and floating window modes

#### Windows
- Uses native Win32 API for optimal integration
- Supports high-DPI displays automatically
- Compatible with Windows 10 and later

#### macOS
- Uses Cocoa framework for native look and feel
- Supports retina displays
- Compatible with macOS 10.15 and later

### Development

To extend the GUI functionality:

1. **Modify graphics rendering** in `render_graphics()` function
2. **Add interactive elements** by handling platform-specific events
3. **Extend CLAP GUI extension** support for additional features
4. **Test across platforms** to ensure compatibility

The GUI implementation serves as a complete example of CLAP GUI extension usage and can be adapted for more complex plugin interfaces.