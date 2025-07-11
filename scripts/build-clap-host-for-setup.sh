#!/bin/bash
# build-clap-host-for-setup.sh - Build CLAP Host for Copilot Setup (with caching support)
set -e

OS_TYPE="$1"
CLAP_HOST_DIR_ARG="${2}"
VCPKG_ROOT="${3:-$(pwd)/vcpkg}"
CACHE_HIT="${4:-false}"

# Devcontainer環境ではCLAP_HOST_DIRのデフォルトを変更
if [ -n "$DEVCONTAINER" ] || [ -d "/opt/clap-host-template" ]; then
    DEFAULT_CLAP_HOST_DIR="/opt/clap-host-template"
else
    DEFAULT_CLAP_HOST_DIR="clap-host-repo"
fi

CLAP_HOST_DIR="${CLAP_HOST_DIR_ARG:-$DEFAULT_CLAP_HOST_DIR}"

echo "=== Building CLAP Host for Setup on $OS_TYPE ==="
echo "CLAP_HOST_DIR: $CLAP_HOST_DIR"
echo "Cache hit: $CACHE_HIT"

if [ "$CACHE_HIT" = "true" ]; then
    echo "✓ CLAP Host build cache hit - skipping build"
    
    # Verify cached build exists
    if [ -d "$CLAP_HOST_DIR/builds" ]; then
        echo "Cached build directory found:"
        ls -la "$CLAP_HOST_DIR/builds/" | head -5
        
        # Find executable in cached build
        CLAP_HOST_EXEC=$(find "$CLAP_HOST_DIR/builds" -name "clap-host*" -type f | head -1)
        if [ -n "$CLAP_HOST_EXEC" ]; then
            echo "✓ Cached CLAP Host executable found: $CLAP_HOST_EXEC"
            chmod +x "$CLAP_HOST_EXEC"
        else
            echo "Warning: No CLAP Host executable found in cache"
        fi
    else
        echo "Warning: Cached build directory not found"
    fi
    
    exit 0
fi

echo "Building CLAP Host from source..."

cd "$CLAP_HOST_DIR"

mkdir -p ../screenshots

case "$OS_TYPE" in
    "linux")
        # Set vcpkg toolchain
        export CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
        
        echo "Using vcpkg at: $VCPKG_ROOT"
        echo "CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
        
        # Simple build approach for setup
        BUILD_SUCCESS=false
        
        echo "Trying simple build with vcpkg toolchain"
        if cmake . -B builds/setup-build -G Ninja \
            -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE" \
            -DCMAKE_BUILD_TYPE=Release; then
            if cmake --build builds/setup-build --config Release; then
                BUILD_SUCCESS=true
                echo "✓ CLAP Host build succeeded"
            else
                echo "CLAP Host build failed"
            fi
        else
            echo "CLAP Host configure failed"
        fi

        # Fallback to simple build without vcpkg
        if [ "$BUILD_SUCCESS" = "false" ]; then
            echo "Trying fallback build without vcpkg"
            if cmake . -B builds/fallback-build -G Ninja -DCMAKE_BUILD_TYPE=Release; then
                if cmake --build builds/fallback-build --config Release; then
                    BUILD_SUCCESS=true
                    echo "✓ Fallback build succeeded"
                else
                    echo "Fallback build failed"
                fi
            else
                echo "Fallback configure failed"
            fi
        fi

        if [ "$BUILD_SUCCESS" = "false" ]; then
            echo "⚠️ CLAP Host build failed - this is acceptable for setup mode"
        fi
        ;;
        
    *)
        echo "CLAP Host build not implemented for $OS_TYPE in setup mode"
        ;;
esac

# Find the built executable
echo "=== Finding CLAP Host executable ==="
EXECUTABLES_FILE="../screenshots/found-executables-$OS_TYPE.txt"
find builds -name "clap-host*" -type f 2>/dev/null | tee "$EXECUTABLES_FILE" || echo "No clap-host executable found"

if [ -s "$EXECUTABLES_FILE" ]; then
    echo "✓ CLAP Host build completed successfully"
else
    echo "⚠️ No CLAP Host executable found - this is acceptable for setup mode"
fi
