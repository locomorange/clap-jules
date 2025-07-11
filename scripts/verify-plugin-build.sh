#!/bin/bash
# verify-plugin-build.sh - Verify that the plugin was built successfully
set -e

OS_TYPE="$1"
BUILD_DIR="${2:-build}"

echo "=== Verifying plugin build for $OS_TYPE ==="

case "$OS_TYPE" in
    "linux")
        PLUGIN_FILE="$BUILD_DIR/MyFirstClapPlugin.so"
        ;;
    "windows")
        PLUGIN_FILE="$BUILD_DIR/Release/MyFirstClapPlugin.clap"
        ;;
    "macos")
        PLUGIN_FILE="$BUILD_DIR/MyFirstClapPlugin.dylib"
        ;;
    *)
        echo "Unknown OS type: $OS_TYPE"
        exit 1
        ;;
esac

echo "Looking for plugin file: $PLUGIN_FILE"

if [ -f "$PLUGIN_FILE" ]; then
    echo "✓ Plugin built successfully: $PLUGIN_FILE"
    ls -la "$PLUGIN_FILE"
    
    # Additional file information if available
    if command -v file &> /dev/null; then
        file "$PLUGIN_FILE"
    fi
    
    echo "success=true" >> ${GITHUB_OUTPUT:-/dev/null}
else
    echo "✗ Error: Plugin file not found!"
    echo "Build directory contents:"
    ls -la "$BUILD_DIR"/ || echo "Build directory not found"
    
    echo "success=false" >> ${GITHUB_OUTPUT:-/dev/null}
    exit 1
fi
