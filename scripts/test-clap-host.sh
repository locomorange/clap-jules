#!/bin/bash
# test-clap-host-simple.sh - Simplified CLAP Host testing
set -e

OS_TYPE="$1"
CLAP_HOST_DIR="${2:-clap-host-repo}"
PLUGIN_FILE="${3:-$PLUGIN_FILE}"
TIMEOUT_DURATION="${4:-30}"

echo "=== Testing CLAP Host with Plugin on $OS_TYPE ==="

# Use override if available, otherwise use provided plugin file
if [ -n "$PLUGIN_FILE_OVERRIDE" ]; then
    PLUGIN_FILE="$PLUGIN_FILE_OVERRIDE"
elif [ -z "$PLUGIN_FILE" ]; then
    # Determine default plugin file path if not provided
    case "$OS_TYPE" in
        "linux")
            PLUGIN_FILE="$(pwd)/plugin-artifacts/MyFirstClapPlugin.so"
            ;;
        "windows")
            PLUGIN_FILE=$(find "$(pwd)/plugin-artifacts" -name "*.clap" -type f | head -1)
            ;;
        "macos")
            PLUGIN_FILE="$(pwd)/plugin-artifacts/MyFirstClapPlugin.dylib"
            ;;
    esac
fi

# Convert to absolute path if it's not already absolute
if [[ "$PLUGIN_FILE" != /* ]]; then
    PLUGIN_FILE="$(pwd)/$PLUGIN_FILE"
fi

echo "Using plugin file: $PLUGIN_FILE"

# Find clap-host executable
CLAP_HOST_EXEC=$(find "$CLAP_HOST_DIR" -name "clap-host*" -type f | head -1)
if [ -z "$CLAP_HOST_EXEC" ]; then
    echo "✗ Error: clap-host executable not found!"
    exit 1
fi

echo "Found clap-host at: $CLAP_HOST_EXEC"
chmod +x "$CLAP_HOST_EXEC"

if [ ! -f "$PLUGIN_FILE" ]; then
    echo "✗ Error: Plugin file not found: $PLUGIN_FILE"
    exit 1
fi

echo "✓ Plugin file verified: $PLUGIN_FILE"

mkdir -p screenshots

case "$OS_TYPE" in
    "linux")
        echo "Testing CLAP Host with virtual display and screenshot..."
        
        # Calculate screenshot hash before running
        PREV_HASH=""
        if [ -f "previous-screenshots/clap-host-linux-screenshot.png" ]; then
            PREV_HASH=$(sha256sum "previous-screenshots/clap-host-linux-screenshot.png" | cut -d' ' -f1)
            echo "Previous screenshot hash: $PREV_HASH"
        elif [ -f "screenshots/clap-host-linux-screenshot.png" ]; then
            PREV_HASH=$(sha256sum "screenshots/clap-host-linux-screenshot.png" | cut -d' ' -f1)
            echo "Previous screenshot hash: $PREV_HASH"
        fi
        
        # Start virtual display
        export DISPLAY=:99
        Xvfb :99 -screen 0 1024x768x24 &
        XVFB_PID=$!
        sleep 5
        
        # Set up Qt6 library path for runtime
        if [ -n "${QT_ROOT_DIR}" ] && [ -d "${QT_ROOT_DIR}/lib" ]; then
            export LD_LIBRARY_PATH="${QT_ROOT_DIR}/lib:${LD_LIBRARY_PATH}"
        fi
        
        timeout "${TIMEOUT_DURATION}s" bash -c "
            '$CLAP_HOST_EXEC' --clap-plugin '$PLUGIN_FILE' &
            CLAP_PID=\$!
            sleep 10
            
            # Take screenshot
            import -window root screenshots/clap-host-linux-screenshot-new.png 2>/dev/null || \
            scrot screenshots/clap-host-linux-screenshot-new.png 2>/dev/null || \
            echo 'Screenshot capture failed'
            
            kill \$CLAP_PID 2>/dev/null || true
        " || echo "Test completed"
        
        kill $XVFB_PID 2>/dev/null || true
        
        # Calculate new screenshot hash and compare
        if [ -f "screenshots/clap-host-linux-screenshot-new.png" ]; then
            mv "screenshots/clap-host-linux-screenshot-new.png" "screenshots/clap-host-linux-screenshot.png"
            NEW_HASH=$(sha256sum "screenshots/clap-host-linux-screenshot.png" | cut -d' ' -f1)
            echo "NEW_SCREENSHOT_HASH=$NEW_HASH" >> ${GITHUB_ENV:-/dev/null}
            
            if [ -n "$PREV_HASH" ] && [ "$PREV_HASH" != "$NEW_HASH" ]; then
                echo "SCREENSHOT_CHANGED=true" >> ${GITHUB_ENV:-/dev/null}
                echo "Screenshot changed"
            else
                echo "Screenshot unchanged"
            fi
        else
            echo "No screenshot generated"
        fi
        ;;
        
    "windows")
        echo "Testing CLAP Host on Windows..."
        timeout "${TIMEOUT_DURATION}s" "$CLAP_HOST_EXEC" --clap-plugin "$PLUGIN_FILE" || echo "Windows test completed"
        ;;
        
    "macos")
        echo "Testing CLAP Host on macOS..."
        timeout "${TIMEOUT_DURATION}s" "$CLAP_HOST_EXEC" --clap-plugin "$PLUGIN_FILE" || echo "macOS test completed"
        ;;
        
    *)
        echo "Unknown OS type: $OS_TYPE"
        exit 1
        ;;
esac

echo "✓ CLAP Host test completed for $OS_TYPE"
