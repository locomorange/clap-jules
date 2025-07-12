#!/bin/bash
# test-clap-host.sh - Test CLAP Host with plugin and take screenshot
set -e

OS_TYPE="$1"
CLAP_HOST_DIR="${2:-clap-host-repo}"
PLUGIN_FILE="${3:-$PLUGIN_FILE}"
TIMEOUT_DURATION="${4:-30}"

echo "=== Testing CLAP Host with Plugin on $OS_TYPE ==="

# Use override if available, otherwise use provided plugin file
if [ -n "$PLUGIN_FILE_OVERRIDE" ]; then
    PLUGIN_FILE="$PLUGIN_FILE_OVERRIDE"
    echo "Using plugin file override: $PLUGIN_FILE"
elif [ -z "$PLUGIN_FILE" ]; then
    # Determine default plugin file path if not provided
    case "$OS_TYPE" in
        "linux")
            PLUGIN_FILE="$(pwd)/plugin-artifacts/MyFirstClapPlugin.so"
            ;;
        "windows")
            if [ -f "$(pwd)/plugin-artifacts/Release/MyFirstClapPlugin.clap" ]; then
                PLUGIN_FILE="$(pwd)/plugin-artifacts/Release/MyFirstClapPlugin.clap"
            elif [ -f "$(pwd)/plugin-artifacts/MyFirstClapPlugin.clap" ]; then
                PLUGIN_FILE="$(pwd)/plugin-artifacts/MyFirstClapPlugin.clap"
            else
                PLUGIN_FILE=$(find "$(pwd)/plugin-artifacts" -name "*.clap" -type f | head -1)
            fi
            ;;
        "macos")
            PLUGIN_FILE="$(pwd)/plugin-artifacts/MyFirstClapPlugin.dylib"
            ;;
    esac
fi

echo "Using plugin file: $PLUGIN_FILE"

# Convert to absolute path if it's not already absolute
if [[ "$PLUGIN_FILE" != /* ]]; then
    PLUGIN_FILE="$(pwd)/$PLUGIN_FILE"
fi

echo "Absolute plugin file path: $PLUGIN_FILE"

# Find clap-host executable
CLAP_HOST_EXEC=$(find "$CLAP_HOST_DIR" -name "clap-host*" -type f | head -1)
if [ -z "$CLAP_HOST_EXEC" ]; then
    echo "✗ Error: clap-host executable not found!"
    echo "Contents of $CLAP_HOST_DIR/builds:"
    find "$CLAP_HOST_DIR" -name "*clap-host*" -type f 2>/dev/null || echo "No clap-host files found"
    exit 1
fi

echo "Found clap-host at: $CLAP_HOST_EXEC"
chmod +x "$CLAP_HOST_EXEC"

echo "CLAP Host details:"
ls -la "$CLAP_HOST_EXEC"
file "$CLAP_HOST_EXEC"
echo "CLAP Host dependencies:"
ldd "$CLAP_HOST_EXEC" | head -10

if [ ! -f "$PLUGIN_FILE" ]; then
    echo "✗ Error: Plugin file not found: $PLUGIN_FILE"
    echo "Current directory: $(pwd)"
    echo "Plugin artifacts directory contents:"
    ls -la plugin-artifacts/ 2>/dev/null || echo "plugin-artifacts directory not found"
    exit 1
fi

echo "✓ Plugin file verified: $PLUGIN_FILE"
echo "Plugin file details:"
ls -la "$PLUGIN_FILE"
file "$PLUGIN_FILE"

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
            echo "Found previous screenshot in current directory"
        else
            echo "No previous screenshot found - this will be the first"
        fi
        
        # Start virtual display
        export DISPLAY=:99
        Xvfb :99 -screen 0 1024x768x24 &
        XVFB_PID=$!
        sleep 5
        
        # Test clap-host and take screenshot
        echo "Debug: CLAP_HOST_EXEC='$CLAP_HOST_EXEC'"
        echo "Debug: PLUGIN_FILE='$PLUGIN_FILE'"
        echo "Debug: Absolute plugin path='$(realpath "$PLUGIN_FILE" 2>/dev/null || echo "File not found")'"
        echo "Debug: Command that will be executed: '$CLAP_HOST_EXEC' '$PLUGIN_FILE'"
        
        # Check CLAP Host help to understand expected arguments
        echo "CLAP Host help/usage:"
        timeout 5s "$CLAP_HOST_EXEC" --help 2>/dev/null || echo "No help available or help command failed"
        
        timeout "${TIMEOUT_DURATION}s" bash -c "
            echo 'Starting CLAP Host with plugin...'
            '$CLAP_HOST_EXEC' '$PLUGIN_FILE' &
            CLAP_PID=\$!
            sleep 10
            
            # Take screenshot
            import -window root screenshots/clap-host-linux-screenshot-new.png 2>/dev/null || \
            scrot screenshots/clap-host-linux-screenshot-new.png 2>/dev/null || \
            echo 'Screenshot capture failed'
            
            kill \$CLAP_PID 2>/dev/null || true
        " || echo "Screenshot capture completed"
        
        kill $XVFB_PID 2>/dev/null || true
        
        # Calculate new screenshot hash and compare
        if [ -f "screenshots/clap-host-linux-screenshot-new.png" ]; then
            mv "screenshots/clap-host-linux-screenshot-new.png" "screenshots/clap-host-linux-screenshot.png"
            NEW_HASH=$(sha256sum "screenshots/clap-host-linux-screenshot.png" | cut -d' ' -f1)
            echo "NEW_SCREENSHOT_HASH=$NEW_HASH" >> ${GITHUB_ENV:-/dev/null}
            
            if [ "$PREV_HASH" != "$NEW_HASH" ]; then
                echo "SCREENSHOT_CHANGED=true" >> ${GITHUB_ENV:-/dev/null}
                echo "✓ Screenshot changed - new hash: $NEW_HASH"
            else
                echo "SCREENSHOT_CHANGED=false" >> ${GITHUB_ENV:-/dev/null}
                echo "Screenshot unchanged"
            fi
        else
            echo "SCREENSHOT_CHANGED=false" >> ${GITHUB_ENV:-/dev/null}
            echo "No screenshot generated"
        fi
        ;;
        
    "macos")
        echo "Testing clap-host functionality without screenshot..."
        timeout "${TIMEOUT_DURATION}s" bash -c "
            '$CLAP_HOST_EXEC' '$PLUGIN_FILE' &
            CLAP_PID=\$!
            sleep 10
            kill \$CLAP_PID 2>/dev/null || true
        " || echo "CLAP Host test completed"
        
        echo "CLAP Host test completed on macOS (no screenshot)" > screenshots/clap-host-test-macos.txt
        echo "SCREENSHOT_CHANGED=false" >> ${GITHUB_ENV:-/dev/null}
        ;;
        
    "windows")
        echo "Testing clap-host functionality without screenshot..."
        timeout "${TIMEOUT_DURATION}s" bash -c "
            '$CLAP_HOST_EXEC' '$PLUGIN_FILE' &
            CLAP_PID=\$!
            sleep 10
            kill \$CLAP_PID 2>/dev/null || true
        " || echo "CLAP Host test completed"
        
        echo "CLAP Host test completed on Windows (no screenshot)" > screenshots/clap-host-test-windows.txt
        echo "SCREENSHOT_CHANGED=false" >> ${GITHUB_ENV:-/dev/null}
        ;;
        
    *)
        echo "Unknown OS type: $OS_TYPE"
        exit 1
        ;;
esac

echo "✓ CLAP Host test completed for $OS_TYPE"
