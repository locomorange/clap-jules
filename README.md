# clap-jules

A CLAP (CLever Audio Plugin) plugin project with integrated testing infrastructure.

## Building

### Prerequisites
- CMake 3.16 or later
- C++17 compatible compiler
- Git (for submodules)

### Build Steps

1. Clone the repository with submodules:
```bash
git clone --recurse-submodules https://github.com/locomorange/clap-jules.git
cd clap-jules
```

2. Build the project:
```bash
mkdir build
cd build
cmake ..
make
```

## Testing

The project includes multiple testing approaches:

### 1. Unit Tests
Basic unit tests using Google Test:
```bash
cd build
ctest
```

### 2. Plugin Integration Testing
A minimal CLAP plugin loader that validates the plugin without requiring a GUI:
```bash
cd build
./clap_plugin_test ./MyFirstClapPlugin.so
```

This test performs:
- Plugin library loading
- CLAP entry point validation
- Plugin factory verification
- Plugin instantiation and initialization
- Basic lifecycle testing (activate, start/stop processing, deactivate)

### 3. CLAP Host Integration (Optional)

The project includes clap-host as a submodule for full integration testing. To build with clap-host support:

1. Install Qt6 dependencies:
```bash
# On Ubuntu/Debian:
sudo apt-get install qt6-base-dev qt6-tools-dev librtaudio-dev librtmidi-dev pkg-config

# On macOS:
brew install qt6 pkgconfig rtaudio rtmidi

# On Arch Linux:
sudo pacman -S qt6-base qt6-tools rtaudio rtmidi pkg-config
```

2. Build with clap-host:
```bash
cd build
cmake .. -DBUILD_CLAP_HOST=ON
make
```

3. Test with clap-host:
```bash
# The clap-host binary will be available in build/clap-host/host/
./clap-host/host/clap-host -p ./MyFirstClapPlugin.so
```

## Plugin Information

- **Plugin ID**: com.example.myplugin
- **Plugin Name**: My First CLAP Plugin
- **Vendor**: My Company
- **Version**: 0.0.1

## Automated Testing for GitHub Copilot Coding Agent

This repository is configured for automated plugin testing through:

1. **Minimal Plugin Test**: `clap_plugin_test` provides headless validation of plugin functionality
2. **CTest Integration**: All tests can be run via `ctest` for CI/CD pipelines
3. **Optional GUI Testing**: clap-host integration for full interactive testing when GUI is available

The minimal test approach ensures that GitHub Copilot Coding Agent can validate plugin functionality without requiring complex GUI dependencies or user interaction.