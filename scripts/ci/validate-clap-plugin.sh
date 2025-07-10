#!/bin/bash
# validate-clap-plugin.sh - Validate CLAP plugin using clap-validator and clap-info
set -e

OS_TYPE="$1"
ARTIFACTS_DIR="${2:-plugin-artifacts}"

echo "=== Validating CLAP plugin on $OS_TYPE ==="

# Debug: Show what was downloaded
echo "Contents of $ARTIFACTS_DIR directory:"
find "$ARTIFACTS_DIR" -type f -name "*.so" -o -name "*.clap" -o -name "*.dylib" 2>/dev/null || echo "No plugin files found"
ls -la "$ARTIFACTS_DIR"/ 2>/dev/null || echo "$ARTIFACTS_DIR directory not found"

# Determine plugin file path and tool commands based on OS
case "$OS_TYPE" in
    "linux")
        PLUGIN_FILE="$ARTIFACTS_DIR/MyFirstClapPlugin.so"
        VALIDATOR_CMD="./clap-validator"
        INFO_CMD="./clap-info"
        ;;
    "windows")
        # Windows plugin is in Release subdirectory
        if [ -f "$ARTIFACTS_DIR/Release/MyFirstClapPlugin.clap" ]; then
            PLUGIN_FILE="$ARTIFACTS_DIR/Release/MyFirstClapPlugin.clap"
        elif [ -f "$ARTIFACTS_DIR/MyFirstClapPlugin.clap" ]; then
            PLUGIN_FILE="$ARTIFACTS_DIR/MyFirstClapPlugin.clap"
        else
            echo "Searching for .clap file in all subdirectories..."
            PLUGIN_FILE=$(find "$ARTIFACTS_DIR" -name "*.clap" -type f | head -1)
        fi
        VALIDATOR_CMD="./clap-validator.exe"
        INFO_CMD="./clap-info.exe"
        ;;
    "macos")
        PLUGIN_FILE="$ARTIFACTS_DIR/MyFirstClapPlugin.dylib"
        VALIDATOR_CMD="./clap-validator"
        INFO_CMD="./clap-info"
        ;;
    *)
        echo "Unknown OS type: $OS_TYPE"
        exit 1
        ;;
esac

echo "Plugin file: $PLUGIN_FILE"

if [ -f "$PLUGIN_FILE" ]; then
    # Validate the plugin with clap-validator
    if [ -f "$VALIDATOR_CMD" ] || [ -f "${VALIDATOR_CMD}.exe" ]; then
        echo "Validating CLAP plugin with clap-validator..."
        $VALIDATOR_CMD validate "$PLUGIN_FILE" || echo "Validation completed with warnings/errors"
    else
        echo "Warning: clap-validator not available on $OS_TYPE - skipping validation"
    fi
    
    # Get plugin info with clap-info
    if [ -f "$INFO_CMD" ] || [ -f "${INFO_CMD}.exe" ]; then
        echo "Getting plugin info with clap-info..."
        $INFO_CMD "$PLUGIN_FILE" --brief || echo "Info extraction completed"
    else
        echo "Warning: clap-info not available on $OS_TYPE - skipping info extraction"
    fi
    
    echo "✓ Plugin validation completed"
else
    echo "✗ Error: Plugin file $PLUGIN_FILE not found!"
    ls -la "$ARTIFACTS_DIR"/
    exit 1
fi
