# clap-jules

A CLAP (CLever Audio Plugin) example plugin built with C++.

## Features

- Basic CLAP plugin implementation
- CMake build system with CLAP SDK integration
- Google Test integration for testing
- GitHub Actions CI/CD pipeline

## Building

```bash
# Clone with submodules
git clone --recurse-submodules https://github.com/locomorange/clap-jules

# Build
cmake . -B build
cmake --build build --config Release
```

## CLAP-Host Integration

This repository provides integration testing capabilities for the CLAP plugin. For detailed information about clap-host integration testing, see [`.github/copilot-instructions.md`](.github/copilot-instructions.md).

## Testing

```bash
# Run unit tests
ctest --test-dir build --output-on-failure
```