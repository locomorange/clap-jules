#!/bin/bash
# build-clap-host.sh - Build CLAP Host for testing
set -e

OS_TYPE="$1"
CLAP_HOST_DIR_ARG="${2}"
VCPKG_ROOT="${3:-$(pwd)/vcpkg}"

# Devcontainer環境ではCLAP_HOST_DIRのデフォルトを変更
if [ -n "$DEVCONTAINER" ] || [ -d "/workspaces/clap-jules/tools/clap-host" ]; then
    DEFAULT_CLAP_HOST_DIR="/workspaces/clap-jules/tools/clap-host"
else
    DEFAULT_CLAP_HOST_DIR="clap-host-repo"
fi

CLAP_HOST_DIR="${CLAP_HOST_DIR_ARG:-$DEFAULT_CLAP_HOST_DIR}"

echo "=== Building CLAP Host on $OS_TYPE ==="
echo "CLAP_HOST_DIR: $CLAP_HOST_DIR"

cd "$CLAP_HOST_DIR"

# Clean up previous builds to ensure fresh build
echo "Cleaning up previous builds..."
rm -rf builds/ || true

mkdir -p ../screenshots

case "$OS_TYPE" in
    "linux"|"macos")
        # Set vcpkg toolchain - 重要！
        export CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
        if [ "$OS_TYPE" = "linux" ]; then
            export VCPKG_TARGET_TRIPLET="x64-linux"
        elif [ "$OS_TYPE" = "macos" ]; then
            export VCPKG_TARGET_TRIPLET="arm64-osx"
        fi
        
        echo "Using vcpkg at: $VCPKG_ROOT"
        echo "CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
        echo "VCPKG_TARGET_TRIPLET: $VCPKG_TARGET_TRIPLET"
        
        # Check Qt6 environment
        echo "Qt6 environment check:"
        echo "QT_ROOT_DIR: ${QT_ROOT_DIR:-not set}"
        echo "LD_LIBRARY_PATH: ${LD_LIBRARY_PATH:-not set}"
        if [ -n "${QT_ROOT_DIR}" ] && [ -d "${QT_ROOT_DIR}/lib" ]; then
            echo "Qt6 libraries found at: ${QT_ROOT_DIR}/lib"
            export LD_LIBRARY_PATH="${QT_ROOT_DIR}/lib:${LD_LIBRARY_PATH}"
            echo "Updated LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
        fi
        
        # Verify vcpkg packages are installed
        if [ -d "$VCPKG_ROOT/installed/$VCPKG_TARGET_TRIPLET" ]; then
            echo "Checking installed vcpkg packages..."
            ls -la "$VCPKG_ROOT/installed/$VCPKG_TARGET_TRIPLET/" | grep -E "(include|lib)" || echo "include/lib directories not found"
            find "$VCPKG_ROOT/installed/$VCPKG_TARGET_TRIPLET" -name "RtMidi.h" 2>/dev/null | head -3 || echo "RtMidi.h not found"
        fi
        
        # Check available presets
        echo "Available CMake presets:"
        cmake --list-presets 2>/dev/null || echo "No presets available"
        
        # Try vcpkg-based build strategies
        BUILD_SUCCESS=false
        
        # Skip ninja-vcpkg preset to force using our libs/vcpkg
        echo "Skipping ninja-vcpkg preset to use our libs/vcpkg toolchain"
        
        # Use manual vcpkg build with our libs/vcpkg
        echo "Trying manual build with libs/vcpkg toolchain"
        
        # Additional Qt6 paths for CMake to find Qt6 installed by GitHub Actions
        QT6_CMAKE_ARGS=""
        if [ -n "${QT_ROOT_DIR}" ] && [ -d "${QT_ROOT_DIR}" ]; then
            echo "Adding Qt6 path hints for CMake: ${QT_ROOT_DIR}"
            QT6_CMAKE_ARGS="-DQt6_DIR=${QT_ROOT_DIR}/lib/cmake/Qt6 -DQt6Core_DIR=${QT_ROOT_DIR}/lib/cmake/Qt6Core -DQt6Widgets_DIR=${QT_ROOT_DIR}/lib/cmake/Qt6Widgets -DQt6Gui_DIR=${QT_ROOT_DIR}/lib/cmake/Qt6Gui"
        fi
        
        if cmake . -B builds/vcpkg-build -G Ninja \
            -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE" \
            -DCMAKE_BUILD_TYPE=Release \
            -DVCPKG_TARGET_TRIPLET="$VCPKG_TARGET_TRIPLET" \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DUsePkgConfig=OFF \
            $QT6_CMAKE_ARGS \
            2>&1 | tee "../screenshots/cmake-vcpkg-configure-log-$OS_TYPE.txt"; then
            echo "vcpkg configuration successful"
            if cmake --build builds/vcpkg-build --config Release \
                2>&1 | tee "../screenshots/cmake-vcpkg-build-log-$OS_TYPE.txt"; then
                BUILD_SUCCESS=true
                echo "✓ Manual vcpkg build succeeded"
            else
                echo "Manual vcpkg build failed"
            fi
        else
            echo "Manual vcpkg configure failed"
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
