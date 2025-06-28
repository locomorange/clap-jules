# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a **CLAP (CLever Audio Plugin) development project** in C++ that serves as a template for creating modern audio plugins. CLAP is an audio plugin standard that's an alternative to VST, AU, and other formats.

## Common Development Commands

### Initial Setup
```bash
git submodule update --init --recursive
```

### Build
```bash
cmake . -B build
cmake --build build --config Release
```

### Test
```bash
ctest --test-dir build --output-on-failure
```

### Validation (Linux only, auto-downloaded in CI)
```bash
clap-validator validate build/MyFirstClapPlugin.so
clap-info --brief build/MyFirstClapPlugin.so
```

## Architecture

### Core Plugin Structure
- **my_plugin.cpp/h**: Main plugin implementation with CLAP callbacks
- **Plugin ID**: `com.example.myplugin`
- **Factory pattern**: Uses `my_plugin_factory` for instantiation
- **Current state**: Pass-through plugin (no audio processing implemented)

### Build System
- **CMake-based** with C++17 standard
- **Cross-platform**: Builds `.so` (Linux), `.clap` (Windows), `.dylib` (macOS)
- **Flexible CLAP SDK resolution**: Supports environment variables, CMake arguments, or submodule paths
- **Output**: `MyFirstClapPlugin` shared library

### Dependencies (Git Submodules)
- **libs/clap/**: CLAP SDK
- **libs/clap-helpers/**: CLAP utility functions
- **libs/googletest/**: Unit testing framework

### Testing
- **Framework**: Google Test integrated with CMake/CTest
- **Location**: test/test_my_plugin.cpp
- **Target**: `my_plugin_tests` executable

### CI/CD
- **Multi-platform** GitHub Actions (Ubuntu, Windows, macOS)
- **Automatic validation** with CLAP tools on Linux builds
- **Artifact uploads** for each platform

## Key Implementation Notes

- Plugin follows CLAP specification with all required callbacks
- Uses proper memory management patterns for plugin lifecycle
- Includes comprehensive logging for debugging
- Extension system ready for future features (parameters, GUI, etc.)
- Install target configured for standard CLAP plugin directories