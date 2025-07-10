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

## CI/CD Scripts

The project includes modularized CI/CD scripts in `scripts/ci/` for improved maintainability:

- **`setup-dependencies.sh`**: Platform-specific dependency installation
- **`setup-clap-tools.sh`**: CLAP validation tools setup
- **`verify-plugin-build.sh`**: Plugin build verification
- **`validate-clap-plugin.sh`**: Plugin validation with CLAP tools
- **`setup-vcpkg.sh`**: vcpkg package manager setup
- **`build-clap-host.sh`**: CLAP Host building for integration tests
- **`test-clap-host.sh`**: Plugin testing with CLAP Host and screenshot capture

See `scripts/ci/README.md` for detailed documentation and usage examples.

## Building

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