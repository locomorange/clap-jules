# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a CLAP (CLever Audio Plugin) development project written in C++ that demonstrates a basic audio plugin implementation using the CLAP specification. The project includes validation tools and testing infrastructure.

## Build System & Commands

### Essential Commands

```bash
# Initialize git submodules (required before first build)
git submodule update --init --recursive

# Configure the build
cmake . -B build

# Build the plugin
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure

# Run a specific test
ctest --test-dir build -R my_plugin_tests --output-on-failure
```

### Plugin Validation (Linux only)

The project includes automated CLAP validation tools that can be used manually:

```bash
# Download and setup validation tools (see .github/workflows/build_and_test.yml for current versions)
clap-validator validate build/MyFirstClapPlugin.so
clap-info build/MyFirstClapPlugin.so --brief
```

## Code Architecture

### Core Structure

- **my_plugin.h**: Plugin interface declarations and basic structure
- **my_plugin.cpp**: Complete CLAP plugin implementation with all required callbacks
- **test/test_my_plugin.cpp**: Google Test-based unit tests

### Plugin Architecture

The CLAP plugin follows the standard CLAP specification pattern:

1. **Plugin Entry Point** (`clap_entry`): Global library initialization/cleanup
2. **Plugin Factory** (`my_plugin_factory`): Creates plugin instances
3. **Plugin Instance** (`my_plugin_t`): Individual plugin state and processing
4. **Callback Functions**: Audio processing, lifecycle management, and host communication

Key components:
- Plugin descriptor with metadata (ID: `com.example.myplugin`)
- Audio processing pipeline in `my_plugin_process()`
- Lifecycle management (init, activate, start_processing, etc.)
- Extension system for additional CLAP features

### Dependencies

- **CLAP SDK**: Core CLAP specification (submodule at `libs/clap`)
- **CLAP Helpers**: Utility library (submodule at `libs/clap-helpers`)
- **Google Test**: Testing framework (submodule at `libs/googletest`)

### Build Outputs

- Linux: `build/MyFirstClapPlugin.so`
- Windows: `build/Release/MyFirstClapPlugin.clap`
- macOS: `build/MyFirstClapPlugin.dylib`

## Development Notes

### CMake Configuration

The build system automatically locates CLAP dependencies via:
1. System-installed CLAP packages
2. Environment variable `CLAP_PATH`
3. Default submodule location (`libs/clap`)

### Testing

- Uses Google Test framework
- Test target name: `my_plugin_tests`
- Tests are discovered automatically via `gtest_discover_tests()`

### CI/CD

GitHub Actions workflow builds and tests on Ubuntu, Windows, and macOS with automated CLAP validation on Linux builds.