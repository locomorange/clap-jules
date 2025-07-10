#!/bin/bash
# build-clap-host.sh - Build CLAP Host for testing
set -e

OS_TYPE="$1"
CLAP_HOST_DIR="${2:-clap-host-repo}"
VCPKG_ROOT="${3:-$(pwd)/vcpkg}"

echo "=== Building CLAP Host on $OS_TYPE ==="

cd "$CLAP_HOST_DIR"

mkdir -p ../screenshots

case "$OS_TYPE" in
    "linux"|"macos")
        # Set vcpkg toolchain
        export CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
        
        echo "Using vcpkg at: $VCPKG_ROOT"
        echo "CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
        
        # Check available presets
        echo "Available CMake presets:"
        cmake --list-presets 2>/dev/null || echo "No presets available"
        
        # Try vcpkg-based build first
        BUILD_SUCCESS=false
        
        echo "Trying manual build with vcpkg toolchain"
        if cmake . -B builds/vcpkg-build -G Ninja \
            -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE" \
            -DCMAKE_BUILD_TYPE=Release; then
            if cmake --build builds/vcpkg-build --config Release; then
                BUILD_SUCCESS=true
                echo "✓ Manual vcpkg build succeeded"
            else
                echo "Manual vcpkg build failed"
            fi
        else
            echo "Manual vcpkg configure failed"
        fi
        
        # Fallback to preset if vcpkg manual build failed
        if [ "$BUILD_SUCCESS" = "false" ]; then
            echo "Trying preset-based build"
            if cmake --preset=default 2>/dev/null; then
                if cmake --build --preset=default 2>/dev/null; then
                    BUILD_SUCCESS=true
                    echo "✓ Preset build succeeded"
                else
                    echo "Preset build failed"
                fi
            else
                echo "Preset configure failed"
            fi
        fi
        
        # Last resort: simple cmake build without specific dependencies
        if [ "$BUILD_SUCCESS" = "false" ]; then
            echo "Trying simple cmake build"
            if cmake . -B builds/simple-build -G Ninja -DCMAKE_BUILD_TYPE=Release; then
                if cmake --build builds/simple-build --config Release; then
                    BUILD_SUCCESS=true
                    echo "✓ Simple build succeeded"
                else
                    echo "Simple build failed"
                fi
            else
                echo "Simple configure failed"
            fi
        fi

        if [ "$BUILD_SUCCESS" = "false" ]; then
            echo "✗ All build attempts failed"
            exit 1
        fi
        ;;
        
    "windows")
        BUILD_SUCCESS=false

        # Try build script first for static build (primary method)
        if [ -f "scripts/build.sh" ]; then
            echo "Trying static build script"
            if bash scripts/build.sh; then
                BUILD_SUCCESS=true
                echo "✓ Static build script succeeded"
            else
                echo "Static build script failed"
            fi
        else
            echo "scripts/build.sh not found, trying other build methods"
        fi

        # Fallback to simple cmake build
        if [ "$BUILD_SUCCESS" = "false" ]; then
            echo "Trying simple cmake build for Windows"
            if cmake . -B builds/windows-build -DCMAKE_BUILD_TYPE=Release; then
                if cmake --build builds/windows-build --config Release; then
                    BUILD_SUCCESS=true
                    echo "✓ Simple Windows build succeeded"
                else
                    echo "Simple Windows build failed"
                fi
            else
                echo "Simple Windows configure failed"
            fi
        fi

        if [ "$BUILD_SUCCESS" = "false" ]; then
            echo "✗ All Windows build attempts failed"
            exit 1
        fi
        ;;
        
    *)
        echo "Unknown OS type: $OS_TYPE"
        exit 1
        ;;
esac

# Find the built executable
echo "=== Finding CLAP Host executable ==="
EXECUTABLES_FILE="../screenshots/found-executables-$OS_TYPE.txt"
find builds -name "clap-host*" -type f 2>/dev/null | tee "$EXECUTABLES_FILE" || echo "No clap-host executable found"

if [ ! -s "$EXECUTABLES_FILE" ]; then
    echo "Warning: No CLAP Host executable found"
else
    echo "✓ CLAP Host build completed successfully"
fi
