# clap-jules

A CLAP (CLever Audio Plugin) development project with integrated Flutter GUI and validation tools.

## Features

- **CLAP Plugin**: Basic audio effect plugin with GUI support
- **Flutter GUI**: Cross-platform desktop interface for plugin controls
- **Cross-platform**: Supports Windows, Mac, and Linux desktop environments
- **Validation Tools**: Automated CLAP plugin validation and analysis

## Requirements

### For CLAP Plugin Development
- CMake 3.16 or later
- C++17 compatible compiler
- Git (for submodules)

### For Flutter GUI (Optional)
- Flutter SDK 3.0 or later
- Platform-specific dependencies:
  - **Linux**: `libgtk-3-dev`, `pkg-config`
  - **Windows**: Visual Studio Build Tools
  - **macOS**: Xcode

## Building

### Basic CLAP Plugin

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

### Flutter GUI

#### Install Flutter (if not already installed)

```bash
# Download Flutter SDK
curl -o flutter_linux.tar.xz https://storage.googleapis.com/flutter_infra_release/releases/stable/linux/flutter_linux_3.24.5-stable.tar.xz
tar xf flutter_linux.tar.xz
export PATH="$PATH:$PWD/flutter/bin"

# Enable desktop support
flutter config --enable-linux-desktop
flutter config --enable-windows-desktop
flutter config --enable-macos-desktop
```

#### Build Flutter App

```bash
# Navigate to Flutter app directory
cd flutter_ui

# Get dependencies
flutter pub get

# Build for your platform
flutter build linux    # Linux
flutter build windows  # Windows  
flutter build macos    # macOS
```

#### Linux Dependencies

```bash
sudo apt update
sudo apt install libgtk-3-dev libblkid-dev pkg-config cmake ninja-build
```

## Usage

### Testing the Plugin

Use the included CLAP validation tools:

```bash
# Get plugin information
./clap-info build/MyFirstClapPlugin.so --brief

# Validate plugin
./clap-validator validate build/MyFirstClapPlugin.so
```

### GUI Integration

The CLAP plugin includes GUI extension support that can launch the Flutter interface:

1. Load the plugin in a CLAP-compatible host
2. Open the plugin's GUI interface
3. The host will display the Flutter-based control interface

### Manual GUI Testing

You can test the Flutter GUI independently:

```bash
cd flutter_ui
flutter run -d linux    # or windows/macos
```

## Architecture

### CLAP Plugin Architecture
- **Core Plugin**: Basic audio effect functionality with CLAP 1.2.6 compatibility
- **GUI Extension**: Implements `clap.gui` extension for host integration
- **Process Management**: Manages Flutter app lifecycle (create/show/hide/destroy)

### Flutter Integration
- **Cross-platform UI**: Single codebase for Linux, Windows, macOS
- **Plugin Controls**: Volume, frequency, and enable/disable controls
- **Real-time Display**: Plugin status and information display

### Communication Flow
1. CLAP host requests GUI extension from plugin
2. Plugin provides GUI extension interface
3. Host calls GUI create/show methods
4. Plugin launches Flutter app as child process
5. Flutter app displays plugin interface
6. Host calls GUI hide/destroy to cleanup

## Troubleshooting

### Common Issues

**Plugin not loading in host:**
- Ensure all dependencies are in the library path
- Check plugin validation with `./clap-validator validate build/MyFirstClapPlugin.so`

**Flutter app not launching:**
- Verify Flutter libraries are in `build/lib/` directory
- Check file permissions on `flutter_ui_app` executable
- Ensure GTK3 development libraries are installed on Linux

**Build failures:**
- Update submodules: `git submodule update --init --recursive`
- Install platform dependencies (see Requirements section)
- Use supported Flutter version (3.0+)

### Development

To modify the Flutter interface:
1. Edit `flutter_ui/lib/main.dart`
2. Rebuild: `cd flutter_ui && flutter build linux`
3. Copy to build directory: `cp build/linux/x64/release/bundle/* ../build/`

To extend the CLAP plugin:
1. Modify `my_plugin.cpp` for audio processing
2. Update `my_plugin.h` for additional state
3. Rebuild: `cmake --build build --config Release`

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