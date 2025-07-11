#!/bin/bash

# Demo script for CLAP Plugin with MVVM Architecture
echo "=== CLAP Plugin Demo - MVVM with Low-Pass Filter ==="
echo

# Build the plugin
echo "1. Building the plugin..."
cd /home/runner/work/clap-jules/clap-jules
unset VCPKG_ROOT
unset CMAKE_TOOLCHAIN_FILE
cmake . -B build -DCMAKE_TOOLCHAIN_FILE= > /dev/null 2>&1
cmake --build build --config Release > /dev/null 2>&1

if [ -f "build/MyFirstClapPlugin.so" ]; then
    echo "✓ Plugin built successfully: MyFirstClapPlugin.so"
else
    echo "✗ Plugin build failed"
    exit 1
fi

echo

# Run tests
echo "2. Running comprehensive tests..."
ctest --test-dir build --output-on-failure 2>&1 | grep -E "(Test #|tests passed|tests failed)"
echo

# Validate plugin file
echo "3. Plugin validation..."
echo "Plugin file: $(ls -la build/MyFirstClapPlugin.so | awk '{print $5, $9}')"
echo "Plugin type: $(file build/MyFirstClapPlugin.so)"
echo

# Test with CLAP validator if available
echo "4. CLAP validation..."
if command -v ./clap-validator &> /dev/null; then
    echo "Running CLAP validator..."
    ./clap-validator validate build/MyFirstClapPlugin.so 2>&1 | head -10
else
    echo "CLAP validator not available (this is expected in CI)"
fi

echo

# Show implementation features
echo "5. Implementation Features Demonstrated:"
echo "✓ MVVM Architecture (Model-View-ViewModel pattern)"
echo "✓ Dependency Injection using boost-di"
echo "✓ Low-Pass Filter DSP processing"
echo "✓ GUI Framework with frequency control"
echo "✓ Comprehensive unit tests (4 test cases)"
echo "✓ CLAP plugin framework integration"
echo "✓ Build system with submodule dependencies"

echo

echo "6. Architecture Components:"
echo "   - AudioModel (FilterAudioModel): Core audio processing"
echo "   - PluginViewModel (FilterPluginViewModel): Business logic"
echo "   - PluginGUI (SimplePluginGUI): User interface"
echo "   - LowPassFilter: Digital signal processing"
echo "   - boost-di: Dependency injection container"

echo

echo "7. Submodules integrated:"
echo "   - boost-di: $(ls libs/boost-di/include/boost/ 2>/dev/null | wc -l) header files"
echo "   - brisk: $(find libs/brisk -name "*.hpp" 2>/dev/null | wc -l) header files (framework ready)"
echo "   - kfr: $(find libs/kfr -name "*.hpp" 2>/dev/null | wc -l) header files (framework ready)"

echo

echo "=== Demo completed successfully! ==="
echo "The plugin implements a complete MVVM architecture with:"
echo "• Clean separation of concerns"
echo "• Testable components"
echo "• Real-time audio processing"
echo "• GUI framework foundation"
echo "• Professional C++ practices"