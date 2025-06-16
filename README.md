# clap-jules

A CLAP (CLever Audio Plugin) development project with integrated validation and analysis tools, featuring Skia 2D graphics support for custom plugin interfaces.

## Features

### Graphics Support 🎨
- **Skia 2D Graphics Integration**: High-quality vector graphics rendering
- **Cross-platform Graphics API**: Consistent interface across all platforms
- **Fallback Implementation**: Works with or without Skia installed
- **Plugin UI Ready**: Foundation for custom knobs, meters, and visualizations

### Development Tools

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

## Graphics Integration

The project includes a sophisticated graphics layer that supports:

- **2D Drawing Operations**: Rectangles, circles, lines, and text rendering
- **State Management**: Save/restore graphics state, transformations
- **Color Management**: RGBA color support with alpha blending
- **Backend Flexibility**: Automatic fallback when Skia is not available

### Quick Graphics Test

```bash
# Build and test graphics functionality
cmake . -B build
cmake --build build --config Release
./clap-info --brief build/MyFirstClapPlugin.so

# You should see graphics backend information in the output
```

For full Skia integration instructions, see [docs/SKIA_INTEGRATION.md](docs/SKIA_INTEGRATION.md).

## Building

```bash
# Initialize submodules
git submodule update --init --recursive

# Basic build (uses stub graphics)
cmake . -B build
cmake --build build --config Release

# Build with Skia support (requires Skia installation)
cmake . -B build -DCLAP_JULES_USE_SKIA=ON
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

The built plugin will be available as `build/MyFirstClapPlugin.so` on Linux or `build/Release/MyFirstClapPlugin.clap` on Windows.

### CMake Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CLAP_JULES_USE_SKIA` | `OFF` | Enable Skia 2D graphics support |
| `CLAP_JULES_BUILD_GRAPHICS_EXAMPLES` | `ON` | Build graphics examples |