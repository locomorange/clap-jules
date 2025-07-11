# clap-jules

A CLAP (CLever Audio Plugin) development project with integrated validation and analysis tools, now featuring Flutter UI integration.

## Features

- **CLAP Audio Plugin**: Full-featured audio plugin implementation
- **Flutter UI Integration**: Modern cross-platform user interface
- **Automated Testing**: Comprehensive test suite with Google Test
- **Development Tools**: Integrated CLAP validation and analysis tools

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

## Flutter UI Integration

The project includes a Flutter-based user interface for the CLAP plugin. The Flutter UI provides:

- Modern, responsive interface design
- Cross-platform compatibility (Linux, Windows, macOS)
- Real-time parameter control
- Plugin state visualization

### Flutter Prerequisites

To build and run the Flutter UI:

1. **Install Flutter SDK** (optional for basic plugin functionality):
   ```bash
   # Using snap (Ubuntu/Linux)
   sudo snap install flutter --classic
   
   # Or download from https://flutter.dev/docs/get-started/install
   ```

2. **Install required dependencies**:
   ```bash
   # Linux desktop development
   sudo apt-get install clang cmake ninja-build pkg-config libgtk-3-dev
   ```

3. **Configure Flutter for desktop** (if building Flutter UI):
   ```bash
   flutter config --enable-linux-desktop
   flutter doctor  # Verify setup
   ```

## Building

```bash
# Initialize submodules
git submodule update --init --recursive

# Configure and build (includes Flutter UI integration)
cmake . -B build
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

### Flutter UI Development

The Flutter UI is located in the `flutter_ui/` directory:

```bash
# Navigate to Flutter UI directory
cd flutter_ui

# Get Flutter dependencies (if Flutter SDK is installed)
flutter pub get

# Run Flutter app in development mode
flutter run -d linux

# Build Flutter app for production
flutter build linux --release
```

## Plugin Features

The built plugin will be available as `build/MyFirstClapPlugin.so` on Linux or `build/Release/MyFirstClapPlugin.clap` on Windows.

### Flutter UI Integration Features:
- **Automatic UI Launch**: Flutter UI launches when plugin is initialized
- **Parameter Control**: Real-time control of gain and bypass parameters
- **Cross-Platform**: Works on Linux, Windows, and macOS desktop environments
- **Modern Interface**: Material Design UI components