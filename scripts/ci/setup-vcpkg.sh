#!/bin/bash
# setup-vcpkg.sh - Setup vcpkg package manager
set -e

OS_TYPE="$1"
VCPKG_DIR="${2:-vcpkg}"

echo "=== Setting up vcpkg for $OS_TYPE ==="

# Clone or update vcpkg
if [ ! -d "$VCPKG_DIR" ] || [ ! -f "$VCPKG_DIR/.git/config" ]; then
    echo "Cloning fresh vcpkg repository..."
    rm -rf "$VCPKG_DIR" 2>/dev/null || true
    git clone https://github.com/Microsoft/vcpkg.git "$VCPKG_DIR"
else
    echo "vcpkg directory exists, updating..."
    cd "$VCPKG_DIR" && git pull && cd ..
fi

# Bootstrap vcpkg
echo "Bootstrapping vcpkg..."
if [ "$OS_TYPE" = "windows" ]; then
    ./"$VCPKG_DIR"/bootstrap-vcpkg.bat
else
    ./"$VCPKG_DIR"/bootstrap-vcpkg.sh
fi

# Determine triplet based on OS
case "$OS_TYPE" in
    "linux")
        TRIPLET="x64-linux"
        ;;
    "windows")
        TRIPLET="x64-windows"
        ;;
    "macos")
        # Detect architecture for macOS
        if [[ $(uname -m) == "arm64" ]]; then
            TRIPLET="arm64-osx"
        else
            TRIPLET="x64-osx"
        fi
        ;;
    *)
        echo "Unknown OS type: $OS_TYPE"
        exit 1
        ;;
esac

echo "Installing RtMidi and RtAudio via vcpkg for $TRIPLET..."
./"$VCPKG_DIR"/vcpkg install rtmidi rtaudio --triplet="$TRIPLET"

# Set environment variables
export VCPKG_ROOT="$(pwd)/$VCPKG_DIR"
export CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

echo "VCPKG_ROOT=$VCPKG_ROOT" >> ${GITHUB_ENV:-/dev/null}
echo "CMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE" >> ${GITHUB_ENV:-/dev/null}

# Add to PATH for current session
echo "$(pwd)/$VCPKG_DIR" >> ${GITHUB_PATH:-/dev/null}

echo "✓ vcpkg setup completed"
echo "VCPKG_ROOT: $VCPKG_ROOT"
echo "CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
