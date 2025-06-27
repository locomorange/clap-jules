# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Development Commands

### Building
```bash
# Initialize git submodules (required first time)
git submodule update --init --recursive

# Configure build
cmake . -B build

# Build the plugin
cmake --build build --config Release
```

### Testing
```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run tests with Release config (Windows)
ctest --test-dir build -C Release --output-on-failure
```

### Plugin Validation
```bash
# Validate plugin (Linux only - requires clap-validator)
clap-validator validate build/MyFirstClapPlugin.so

# Get plugin information (Linux only - requires clap-info)
clap-info build/MyFirstClapPlugin.so --brief
```

### Output Files
- Linux: `build/MyFirstClapPlugin.so`
- Windows: `build/Release/MyFirstClapPlugin.clap`
- macOS: `build/MyFirstClapPlugin.dylib`

## Architecture

This is a CLAP (CLever Audio Plugin) project implementing a modern audio effect plugin using MVVM (Model-View-ViewModel) architecture with Dependency Injection.

### Core Structure
- **Plugin Entry Point**: `clap_entry` structure in `my_plugin.cpp:185` - main entry point for CLAP host
- **Plugin Factory**: `my_plugin_factory` structure provides plugin creation and metadata
- **Plugin Instance**: `my_plugin_t` struct contains plugin state, CLAP interface, and MVVM components

### MVVM Architecture Components

#### Model Layer (`include/audio/audio_processor.hpp`)
- `IAudioProcessor`: Interface for audio processing
- `AudioProcessor`: Base implementation for audio processing
- `AudioBuffer`: Wrapper for audio data

#### View Layer (`include/plugin/plugin_window.hpp`)
- `PluginWindow`: Brisk-based GUI window
- Platform-specific window embedding for DAW integration
- UI controls bound to ViewModel properties

#### ViewModel Layer (`include/plugin/plugin_view_model.hpp`)
- `PluginViewModel`: Business logic and state management
- `Property<T>`: Observable properties with change notifications
- `RelayCommand`: Command pattern implementation
- Bridges between audio processing and UI

#### Dependency Injection (`include/di/container.hpp`)
- `Container`: Service container using Boost.DI
- `ServiceConfiguration`: DI configuration and service creation
- Manages component lifecycles and dependencies

### Key Components

#### Plugin Descriptor (`my_plugin.cpp:22`)
Defines plugin metadata including ID, name, vendor, and features array.

#### Plugin Lifecycle Functions
- `my_plugin_init/destroy`: Plugin instance creation/cleanup with MVVM initialization
- `my_plugin_activate/deactivate`: Resource allocation and ViewModel configuration
- `my_plugin_start_processing/stop_processing`: Processing state management
- `my_plugin_reset`: State reset through ViewModel

#### Audio Processing (`my_plugin.cpp:167`)
- Processing delegated to ViewModel's `processAudio` method
- MVVM pattern separates audio logic from CLAP interface
- Error handling with proper status codes

#### GUI Extension (`my_plugin.cpp:206`)
- Complete CLAP GUI extension implementation
- Brisk-based window management
- Cross-platform window API support (Win32, Cocoa, X11)
- Resizable window with size constraints

### Dependencies
- **CLAP SDK**: Located in `libs/clap/` (git submodule)
- **CLAP Helpers**: Optional helpers in `libs/clap-helpers/` (git submodule)
- **GoogleTest**: Test framework in `libs/googletest/` (git submodule)
- **KFR**: Digital signal processing library in `libs/kfr/` (git submodule)
- **Brisk**: Cross-platform GUI framework in `libs/brisk/` (git submodule)
- **Boost.DI**: Dependency injection library in `libs/boost-di/` (git submodule)

### CMake Configuration
- Plugin built as shared library (`my_clap_plugin` target)
- Platform-specific output naming (.clap on Windows, .so/.dylib on Unix)
- Automatic CLAP SDK discovery (environment/cmake variable/submodule)
- Google Test integration with `gtest_discover_tests`

### CI/CD Integration
The GitHub Actions workflow automatically:
- Downloads CLAP validator (v0.3.2) and CLAP info (v1.2.2) tools on Linux
- Validates built plugins for CLAP compliance
- Builds on Ubuntu, Windows, and macOS
- Uploads platform-specific artifacts