# CLAP Plugin with MVVM Architecture, Dependency Injection, and DSP

This CLAP plugin demonstrates a modern architecture using:

## Features Implemented

### 1. MVVM (Model-View-ViewModel) Pattern
- **AudioModel**: Handles the core audio processing logic
- **PluginViewModel**: Mediates between the model and view
- **PluginView/GUI**: User interface components (currently simplified)

### 2. Dependency Injection with boost-di
- Uses boost-di for clean dependency management
- Automatic injection of AudioModel into ViewModel
- Fallback implementation when boost-di is not available

### 3. Low-Pass Filter DSP
- Simple first-order low-pass filter implementation
- Configurable frequency and sample rate
- Framework ready for KFR integration (advanced DSP library)

### 4. GUI Framework
- Abstract GUI interface ready for brisk integration
- Current simple implementation with console output
- Frequency control with callback mechanism
- Show/hide functionality

## Architecture Overview

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Plugin GUI    │────│  PluginViewModel │────│   AudioModel    │
│  (SimpleGUI)    │    │ (FilterPlugin-   │    │ (FilterAudio-   │
│                 │    │  ViewModel)      │    │  Model)         │
└─────────────────┘    └──────────────────┘    └─────────────────┘
         │                        │                        │
         │                        │                        │
         │                        │               ┌─────────────────┐
         │                        │               │   LowPassFilter │
         │                        │               │                 │
         │                        │               └─────────────────┘
         │                        │
         │               ┌──────────────────┐
         │               │   boost-di       │
         │               │   Injector       │
         │               └──────────────────┘
         │
┌─────────────────┐
│   CLAP Plugin   │
│   (my_plugin_t) │
└─────────────────┘
```

## Dependencies

### Core Dependencies
- **CLAP**: Audio plugin framework
- **boost-di**: Dependency injection library
- **googletest**: Unit testing framework

### Future Integrations
- **brisk**: GUI framework for rich user interfaces
- **kfr**: High-performance DSP library for advanced filtering

## Build Instructions

```bash
# Configure with proper environment
unset VCPKG_ROOT
unset CMAKE_TOOLCHAIN_FILE
cmake . -B build -DCMAKE_TOOLCHAIN_FILE=

# Build
cmake --build build --config Release

# Test
ctest --test-dir build --output-on-failure
```

## Testing

The plugin includes comprehensive unit tests:

1. **FilterTest**: Tests low-pass filter functionality
2. **MVVMTest**: Tests ViewModel behavior
3. **GUITest**: Tests GUI interaction and callbacks
4. **MyPluginTest**: Basic sanity checks

## Usage

The plugin processes audio through the MVVM architecture:

1. **Audio Input** → **ViewModel** → **AudioModel** → **LowPassFilter** → **Audio Output**
2. **GUI Changes** → **ViewModel** → **AudioModel** (frequency updates)
3. **Host Callbacks** → **Plugin** → **ViewModel** (sample rate, activation)

## Key Features

### Frequency Control
- Default frequency: 1000 Hz
- Adjustable via GUI interface
- Real-time updates during audio processing

### MVVM Benefits
- Clean separation of concerns
- Testable architecture
- Easy to extend with new features
- Framework-independent core logic

### Dependency Injection Benefits
- Loose coupling between components
- Easy mocking for unit tests
- Configurable component lifetime
- Cleaner initialization code

## Future Enhancements

1. **Brisk GUI Integration**: Rich native GUI with knobs, sliders, and visualizations
2. **KFR DSP Integration**: High-quality filters with SIMD optimization
3. **Parameter Automation**: CLAP parameter support for DAW automation
4. **Preset Management**: Save/load filter configurations
5. **Real-time Visualization**: Frequency response plots

## File Structure

```
├── my_plugin.cpp/h      # Main CLAP plugin implementation
├── filter.cpp/h         # Low-pass filter implementation
├── mvvm.h               # MVVM interface definitions
├── mvvm_impl.h          # MVVM concrete implementations
├── gui.h                # GUI interface and simple implementation
├── test/                # Unit tests
└── libs/                # Third-party dependencies
    ├── clap/           # CLAP framework
    ├── boost-di/       # Dependency injection
    ├── brisk/          # GUI framework (future)
    ├── kfr/            # DSP library (future)
    └── googletest/     # Testing framework
```

This implementation provides a solid foundation for a professional audio plugin with modern C++ practices and clean architecture.