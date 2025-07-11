#!/bin/bash
# verify-plugin-artifact.sh - Verify plugin artifact and set environment variables
set -e

OS_TYPE="$1"
ARTIFACTS_DIR="${2:-plugin-artifacts}"

echo "=== Verifying plugin artifact for $OS_TYPE ==="
echo "Current working directory: $(pwd)"
echo "Contents of $ARTIFACTS_DIR directory:"
ls -la "$ARTIFACTS_DIR"/ || echo "$ARTIFACTS_DIR directory not found"

# Determine expected plugin file location
case "$OS_TYPE" in
    "linux")
        PLUGIN_FILE="$ARTIFACTS_DIR/MyFirstClapPlugin.so"
        ;;
    "windows")
        # Check multiple possible locations for Windows
        if [ -f "$ARTIFACTS_DIR/Release/MyFirstClapPlugin.clap" ]; then
            PLUGIN_FILE="$ARTIFACTS_DIR/Release/MyFirstClapPlugin.clap"
        elif [ -f "$ARTIFACTS_DIR/MyFirstClapPlugin.clap" ]; then
            PLUGIN_FILE="$ARTIFACTS_DIR/MyFirstClapPlugin.clap"
        else
            echo "Searching for .clap file in $ARTIFACTS_DIR..."
            PLUGIN_FILE=$(find "$ARTIFACTS_DIR" -name "*.clap" -type f | head -1)
        fi
        ;;
    "macos")
        PLUGIN_FILE="$ARTIFACTS_DIR/MyFirstClapPlugin.dylib"
        ;;
    *)
        echo "Unknown OS type: $OS_TYPE"
        exit 1
        ;;
esac

echo "Expected plugin file: $PLUGIN_FILE"

if [ -f "$PLUGIN_FILE" ]; then
    echo "✓ Plugin artifact found successfully"
    ls -la "$PLUGIN_FILE"
    
    # Additional file information if available
    if command -v file &> /dev/null; then
        file "$PLUGIN_FILE" || echo "file command failed"
    fi
    
    # Export plugin file path for use in subsequent steps
    echo "PLUGIN_FILE=$PLUGIN_FILE" >> ${GITHUB_ENV:-/dev/null}
else
    echo "✗ Error: Plugin file not found at expected location!"
    echo "Contents of $ARTIFACTS_DIR directory:"
    find "$ARTIFACTS_DIR" -type f 2>/dev/null || echo "No files found in $ARTIFACTS_DIR"
    
    # Try to find any plugin file
    echo "Looking for any plugin files..."
    FOUND_PLUGIN=$(find "$ARTIFACTS_DIR" -name "*.so" -o -name "*.clap" -o -name "*.dylib" 2>/dev/null | head -1)
    if [ -n "$FOUND_PLUGIN" ]; then
        echo "Found plugin file: $FOUND_PLUGIN"
        echo "PLUGIN_FILE_OVERRIDE=$FOUND_PLUGIN" >> ${GITHUB_ENV:-/dev/null}
        echo "PLUGIN_FILE=$FOUND_PLUGIN" >> ${GITHUB_ENV:-/dev/null}
    else
        exit 1
    fi
fi

echo "✓ Plugin artifact verification completed"
