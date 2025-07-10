#!/bin/bash
# verify-plugin-build-optional.sh - Optional plugin build verification (non-failing)
set +e  # Don't exit on errors

OS_TYPE="$1"
BUILD_DIR="${2:-build}"

echo "=== Optional plugin build verification for $OS_TYPE ==="
echo "This is for environment verification only - failures are expected"

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
        exit 0  # Don't fail on unknown OS
        ;;
esac

echo "Looking for plugin file: $PLUGIN_FILE"

if [ -f "$PLUGIN_FILE" ]; then
    echo "✓ Plugin built successfully (bonus!): $PLUGIN_FILE"
    ls -la "$PLUGIN_FILE"
    
    # Additional file information if available
    if command -v file &> /dev/null; then
        file "$PLUGIN_FILE" || echo "file command failed"
    fi
    
    echo "success=true" >> ${GITHUB_OUTPUT:-/dev/null}
else
    echo "Plugin not built - this is normal for setup-only mode"
    
    # Show what's in the build directory for debugging
    if [ -d "$BUILD_DIR" ]; then
        echo "Build directory contents:"
        ls -la "$BUILD_DIR"/ || echo "Build directory listing failed"
    else
        echo "Build directory not found - this is expected in setup-only mode"
    fi
    
    echo "success=false" >> ${GITHUB_OUTPUT:-/dev/null}
fi

echo "✓ Optional plugin build verification completed (non-failing)"
exit 0  # Always exit successfully
