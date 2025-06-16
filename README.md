# clap-jules

A CLAP (CLever Audio Plugin) development project with integrated validation and analysis tools.

## Development Tools

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

## Building

### Basic Build

```bash
# Initialize submodules
git submodule update --init --recursive

# Configure and build
cmake . -B build
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

The built plugin will be available as `build/MyFirstClapPlugin.so` on Linux or `build/Release/MyFirstClapPlugin.clap` on Windows.

### Building with Skia Graphics Library (Optional)

This project supports optional integration with Google's Skia graphics library for advanced rendering capabilities.

#### Prerequisites for Skia Integration

1. **Install depot_tools** (required for Skia):
   ```bash
   git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
   export PATH="${PWD}/depot_tools:${PATH}"
   ```

2. **Install Skia dependencies**:
   - Python 3.8+
   - Git
   - Build tools (GCC/Clang on Linux, Visual Studio on Windows)

#### Setting up Skia

```bash
# Run the Skia setup script (this will take some time)
./scripts/setup_skia.sh
```

This script will:
- Clone the Skia repository to `third_party/skia`
- Download and sync all Skia dependencies
- Configure Skia with optimal settings for audio plugin development
- Build Skia libraries

#### Building with Skia Enabled

```bash
# Configure with Skia enabled
cmake . -B build -DENABLE_SKIA=ON

# Build
cmake --build build --config Release

# Run tests (including Skia integration tests)
ctest --test-dir build --output-on-failure
```

#### Skia Integration Notes

- Skia integration is **completely optional** - the plugin builds and works without it
- When enabled, the plugin includes Skia headers and links against Skia libraries
- Skia adds significant build time and dependencies, so only enable if you need graphics capabilities
- The first Skia build can take 30+ minutes depending on your system
- Skia binaries are quite large (~100MB+), so they are excluded from version control