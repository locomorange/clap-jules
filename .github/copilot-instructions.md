# CLAP Plugin Testing with clap-validator and clap-host

This document provides instructions for testing the clap-jules CLAP audio plugin using automated validation and interactive host tools in CI environments and during development.

## Plugin Information

- **Plugin Name**: My First CLAP Plugin
- **Plugin ID**: `com.example.myplugin`
- **Binary Name**: `MyFirstClapPlugin.so` (Linux) / `MyFirstClapPlugin.clap` (Windows)
- **Vendor**: My Company
- **Version**: 0.0.1

## Prerequisites

### Option 1: Using clap-validator (Recommended for CI)

clap-validator is an automated test suite for CLAP plugins, ideal for CI environments.

#### Installing clap-validator in CI

```yaml
- name: Download and install clap-validator
  run: |
    # Download latest clap-validator binary
    curl -L https://github.com/free-audio/clap-validator/releases/latest/download/clap-validator-ubuntu-latest.tar.gz | tar xz
    chmod +x clap-validator
    sudo mv clap-validator /usr/local/bin/
```

#### Installing clap-validator locally

```bash
# Install Rust if not already installed
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source ~/.cargo/env

# Clone and build clap-validator
git clone https://github.com/robbert-vdh/clap-validator.git
cd clap-validator
cargo build --release
# Binary will be at target/release/clap-validator
```

### Option 2: Using clap-host (For interactive testing)

clap-host provides a GUI interface for testing plugins interactively.

#### Building clap-host in CI (Ubuntu with Qt6)

```yaml
- name: Install Qt6 and dependencies
  run: |
    sudo apt-get update
    sudo apt-get install -y qt6-base-dev qt6-tools-dev cmake ninja-build

- name: Clone and build clap-host
  run: |
    git clone https://github.com/free-audio/clap-host.git
    cd clap-host
    git submodule update --init --recursive
    cmake --preset ninja-system
    cmake --build --preset ninja-system
```

#### Local Development Setup for clap-host

```bash
# Install Qt6 (Ubuntu/Debian)
sudo apt-get install qt6-base-dev qt6-tools-dev cmake ninja-build

# Clone clap-host
git clone --recurse-submodules https://github.com/free-audio/clap-host.git
cd clap-host

# Build clap-host
cmake --preset ninja-system
cmake --build --preset ninja-system
```

## Plugin Testing Commands

### Using clap-validator (Recommended)

#### Basic Plugin Validation

Run comprehensive automated tests on the plugin:

```bash
# Basic validation
clap-validator validate ./build/MyFirstClapPlugin.so

# Show only failed tests (useful for CI)
clap-validator validate ./build/MyFirstClapPlugin.so --only-failed

# Validate with verbose output
clap-validator validate ./build/MyFirstClapPlugin.so --verbose
```

#### List Available Tests

```bash
# List all available test cases
clap-validator list tests

# Run specific test
clap-validator validate --test-filter "Plugin instantiation" ./build/MyFirstClapPlugin.so
```

#### Plugin Information Extraction

```bash
# List all system-installed CLAP plugins
clap-validator list plugins

# List all system-installed plugins in JSON format
clap-validator list plugins --json

# List available presets for all system plugins
clap-validator list presets

# Run full validation to see plugin details and test results
clap-validator validate ./build/MyFirstClapPlugin.so

# Get plugin info in JSON format
clap-validator validate ./build/MyFirstClapPlugin.so --json
```

### Using clap-host (Interactive Testing)

#### Basic Plugin Loading Test

Test if the plugin loads correctly in a GUI environment:

```bash
# Using built clap-host from source
./clap-host/build/Release/clap-host

# Load plugin via command line (if supported)
./clap-host/build/Release/clap-host --plugin ./build/MyFirstClapPlugin.so
```

#### Plugin Discovery

```bash
# The clap-host GUI allows you to:
# 1. Browse and load CLAP plugins
# 2. Test audio processing
# 3. Manipulate parameters
# 4. Test MIDI input/output
```

## CI Integration Examples

### Test Script for CI (using clap-validator)

Create a test script that can be run in CI:

```bash
#!/bin/bash
set -e

echo "Testing clap-jules plugin with clap-validator..."

PLUGIN_PATH="./build/MyFirstClapPlugin.so"

# Test 1: Basic validation
echo "Test 1: Plugin validation"
clap-validator validate $PLUGIN_PATH --only-failed || exit 1

# Test 2: Plugin info extraction
echo "Test 2: Plugin information"
clap-validator validate $PLUGIN_PATH | head -20 || exit 1

# Test 3: Specific test cases
echo "Test 3: Plugin instantiation test"
clap-validator validate --test-filter "Plugin instantiation" $PLUGIN_PATH || exit 1

echo "All tests passed!"
```

### GitHub Actions Workflow Integration

Add to your `.github/workflows/build_and_test.yml`:

```yaml
- name: Test plugin with clap-validator
  run: |
    # Download clap-validator
    curl -L https://github.com/free-audio/clap-validator/releases/latest/download/clap-validator-ubuntu-latest.tar.gz | tar xz
    chmod +x clap-validator
    
    # Run validation tests
    ./clap-validator validate ./build/MyFirstClapPlugin.so --only-failed
    
    # List plugin information
    ./clap-validator validate ./build/MyFirstClapPlugin.so | head -20
    
    # Run specific critical tests
    ./clap-validator validate --test-filter "Plugin instantiation" ./build/MyFirstClapPlugin.so

- name: Test plugin with clap-host (optional GUI tests)
  if: false  # Enable only if GUI testing is needed
  run: |
    # Install Qt6
    sudo apt-get update
    sudo apt-get install -y qt6-base-dev qt6-tools-dev cmake ninja-build
    
    # Build clap-host
    git clone https://github.com/free-audio/clap-host.git
    cd clap-host
    git submodule update --init --recursive
    cmake --preset ninja-system
    cmake --build --preset ninja-system
    
    # Note: GUI tests would require a virtual display (xvfb)
    echo "clap-host built successfully"
```

### Alternative CI Integration with Pre-built Binaries

```yaml
- name: Test plugin with pre-built clap-validator
  run: |
    # Download specific version
    VALIDATOR_VERSION="0.3.0"  # Replace with desired version
    wget https://github.com/free-audio/clap-validator/releases/download/v${VALIDATOR_VERSION}/clap-validator-ubuntu-latest.tar.gz
    tar xzf clap-validator-ubuntu-latest.tar.gz
    chmod +x clap-validator
    
    # Run comprehensive validation
    ./clap-validator validate ./build/MyFirstClapPlugin.so
```

## Common Usage Patterns

### Plugin Discovery and Scanning

```bash
# List all system-installed CLAP plugins
clap-validator list plugins

# Get plugin details through validation (shows plugin info when verbose)
clap-validator validate ./build/MyFirstClapPlugin.so

# List all available test cases
clap-validator list tests

# List presets for all system plugins
clap-validator list presets
```

### Debugging Plugin Issues

```bash
# Run with verbose output for debugging
clap-validator validate ./build/MyFirstClapPlugin.so

# Run specific test in-process for debugging
clap-validator validate --in-process --test-filter "Plugin instantiation" ./build/MyFirstClapPlugin.so

# Use debugger with specific test
gdb --args clap-validator validate --in-process --test-filter "Audio processing" ./build/MyFirstClapPlugin.so
```

### Performance and Stability Testing

```bash
# Run only performance-related tests
clap-validator validate --test-filter "Performance" ./build/MyFirstClapPlugin.so

# Run stress tests
clap-validator validate --test-filter "Stress" ./build/MyFirstClapPlugin.so

# Check for memory leaks and crashes
clap-validator validate --test-filter "Stability" ./build/MyFirstClapPlugin.so
```

## Example Test Output

When running clap-validator on the clap-jules plugin, you might see output like this:

```bash
$ clap-validator validate ./build/MyFirstClapPlugin.so --only-failed
Plugin tests:

 - com.example.myplugin

   - features-categories: The plugin needs to have at least one of the main CLAP category features.
     FAILED: The plugin needs to have at least one of thw following plugin category features: "instrument", "audio-
       effect", "note-effect", or "analyzer".

21 tests run, 6 passed, 1 failed, 14 skipped, 0 warnings
```

This indicates that the plugin needs to specify its category in the plugin descriptor's features array.

## Error Handling

Common issues and solutions:

### clap-validator Issues

1. **Plugin not found**: Ensure the plugin file path is correct and the file exists
   ```bash
   ls -la ./build/MyFirstClapPlugin.so
   ```

2. **Permission denied**: Check file permissions
   ```bash
   chmod +x ./build/MyFirstClapPlugin.so
   ```

3. **Plugin fails to load**: Check for missing dependencies
   ```bash
   ldd ./build/MyFirstClapPlugin.so  # Linux
   otool -L ./build/MyFirstClapPlugin.dylib  # macOS
   ```

4. **Tests fail**: Run with verbose output to see detailed error messages
   ```bash
   clap-validator validate ./build/MyFirstClapPlugin.so --verbose
   ```

### clap-host Issues

1. **Qt6 not found**: Install Qt6 development libraries
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qt6-base-dev qt6-tools-dev
   
   # macOS
   brew install qt6
   
   # Windows
   # Download Qt6 from qt.io
   ```

2. **Display issues in CI**: Use virtual display for GUI testing
   ```bash
   sudo apt-get install xvfb
   xvfb-run -a ./clap-host/build/Release/clap-host
   ```

## Advanced Testing

### Memory Safety Testing

```bash
# Run with Valgrind (Linux)
valgrind --leak-check=full --track-origins=yes \
         clap-validator validate ./build/MyFirstClapPlugin.so --in-process

# Run with AddressSanitizer (requires rebuilding clap-validator with ASAN)
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 \
clap-validator validate ./build/MyFirstClapPlugin.so --in-process
```

### Cross-platform Testing

```bash
# Linux
clap-validator validate ./build/MyFirstClapPlugin.so

# Windows (using WSL or native Windows with clap-validator.exe)
clap-validator.exe validate ./build/Release/MyFirstClapPlugin.clap

# macOS
clap-validator validate ./build/MyFirstClapPlugin.dylib
```

### Automated Testing with Different Configurations

```bash
# Test with different sample rates
for rate in 44100 48000 96000; do
  echo "Testing at ${rate}Hz"
  clap-validator validate --test-filter "Audio processing" ./build/MyFirstClapPlugin.so
done

# Test with different buffer sizes
for size in 64 128 256 512 1024; do
  echo "Testing with buffer size ${size}"
  clap-validator validate --test-filter "Buffer size" ./build/MyFirstClapPlugin.so
done
```

### Integration with Build Systems

#### CMake Integration

Add to your `CMakeLists.txt`:

```cmake
# Add custom test target
if(CLAP_VALIDATOR_PATH)
    add_custom_target(validate_plugin
        COMMAND ${CLAP_VALIDATOR_PATH} validate $<TARGET_FILE:my_clap_plugin> --only-failed
        DEPENDS my_clap_plugin
        COMMENT "Validating CLAP plugin"
    )
endif()
```

#### CTest Integration

```cmake
if(CLAP_VALIDATOR_PATH)
    add_test(NAME clap_plugin_validation
             COMMAND ${CLAP_VALIDATOR_PATH} validate $<TARGET_FILE:my_clap_plugin> --only-failed)
    
    add_test(NAME clap_plugin_info
             COMMAND ${CLAP_VALIDATOR_PATH} info $<TARGET_FILE:my_clap_plugin>)
endif()
```

## Notes for Copilot

- **Primary tool**: Use `clap-validator` for automated testing and CI integration - it's specifically designed for this purpose
- **Plugin ID**: Always use the exact plugin ID `com.example.myplugin` when referencing this plugin
- **Plugin binary**: The plugin binary name is `MyFirstClapPlugin` without extension; extensions are platform-specific (`.so`, `.clap`, `.dylib`)
- **File paths**: Plugin path should be relative to the build directory: `./build/MyFirstClapPlugin.so`
- **CI setup**: clap-validator can be downloaded as pre-built binaries, making CI setup simple
- **GUI testing**: clap-host requires Qt6 and is more suitable for interactive testing than CI
- **Error handling**: Always use `--only-failed` flag in CI to reduce noise
- **Debugging**: Use `--in-process` flag with clap-validator for debugging specific issues
- **Cross-platform**: Both tools support Windows, macOS, and Linux
- **Memory testing**: Combine clap-validator with Valgrind or AddressSanitizer for memory safety

### Quick Reference Commands

```bash
# Essential CI commands
clap-validator validate ./build/MyFirstClapPlugin.so --only-failed
clap-validator validate ./build/MyFirstClapPlugin.so | head -20

# Debugging commands
clap-validator validate ./build/MyFirstClapPlugin.so
clap-validator validate --in-process --test-filter "test-name" ./build/MyFirstClapPlugin.so

# Discovery commands
clap-validator list plugins
clap-validator list tests
clap-validator list presets
```