# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a CLAP (CLever Audio Plugin) development project that implements a basic audio effect plugin. The project uses C++ with CMake build system and includes automated validation and testing tools.

## Development Commands

### Building
```bash
# Initialize submodules (required first time)
git submodule update --init --recursive

# Configure and build
cmake . -B build
cmake --build build --config Release
```

### Testing
```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run tests with specific config (Windows)
ctest --test-dir build -C Release --output-on-failure
```

### CLAP Validation (Linux only)
The project includes automated CLAP validation tools that can be run manually:
```bash
# Validate plugin
clap-validator validate build/MyFirstClapPlugin.so

# Get plugin information
clap-info build/MyFirstClapPlugin.so --brief
```

### Plugin Output Locations
- **Linux**: `build/MyFirstClapPlugin.so`
- **Windows**: `build/Release/MyFirstClapPlugin.clap`
- **macOS**: `build/MyFirstClapPlugin.dylib`

## Architecture

### Core Plugin Structure
- `my_plugin.h`: Plugin structure definition and basic declarations
- `my_plugin.cpp`: Complete CLAP plugin implementation with factory pattern
- `test/test_my_plugin.cpp`: Google Test-based unit tests

### Plugin Implementation Details
The plugin follows the standard CLAP architecture:

1. **Plugin Descriptor**: Defines plugin metadata (ID: "com.example.myplugin")
2. **Plugin Factory**: Creates plugin instances and manages lifecycle
3. **Plugin Entry Point**: Main library entry point (`clap_entry`)
4. **Audio Processing**: Implements the standard CLAP audio processing pipeline

### Key Components
- **CMake Configuration**: Handles CLAP SDK integration via submodules or system packages
- **Submodules**: Includes CLAP SDK, clap-helpers, and Google Test
- **CI/CD**: Multi-platform builds with automatic validation on Ubuntu

### Dependencies
- CLAP SDK (libs/clap)
- CLAP Helpers (libs/clap-helpers) - optional utilities
- Google Test (libs/googletest) - for unit testing
- Brisk Library (libs/brisk) - optional UI framework

### Brisk Library Integration
The project includes Brisk library as an optional dependency for UI development:

```bash
# Enable Brisk in build (requires CMake 3.22+ and additional system dependencies)
cmake . -B build -DENABLE_BRISK=ON

# Install required system dependencies (Ubuntu):
sudo apt-get install -y libuv1-dev libvulkan-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

**Note**: Brisk integration is disabled by default due to complex dependency management requirements. Enable only when needed for UI development.

## Development Notes

- The plugin currently implements basic structure with placeholder audio processing
- GitHub Actions workflow includes automatic CLAP validation and artifact upload
- Project supports cross-platform builds (Ubuntu, Windows, macOS)
- CMake automatically locates CLAP SDK from submodules or environment variable `CLAP_PATH`
- Brisk library provides cross-platform UI framework but requires additional setup