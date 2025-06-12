# clap-jules

A CLAP (CLever Audio Plugin) example plugin built with C++.

## Features

- Basic CLAP plugin implementation
- CMake build system with CLAP SDK integration
- Google Test integration for testing
- GitHub Actions CI/CD pipeline
- **clap-host Integration Testing** - Automated workflow to test plugin loading with the official clap-host

## Building

```bash
# Clone with submodules
git clone --recurse-submodules https://github.com/locomorange/clap-jules

# Build
cmake . -B build
cmake --build build --config Release
```

## CLAP-Host Integration

This repository includes a complete workflow (`.github/workflows/copilot-setup-steps.yml`) that:

1. Builds the clap-jules plugin
2. Clones and builds [free-audio/clap-host](https://github.com/free-audio/clap-host)
3. Tests plugin loading functionality
4. Works with both GitHub-hosted and self-hosted runners

See the workflow file for complete integration testing setup.

## Testing

```bash
# Run unit tests
ctest --test-dir build --output-on-failure

# Test plugin with clap-host (after building both)
./clap-host/builds/*/host/*/clap-host -p build/MyFirstClapPlugin.so
```