#!/bin/bash
# setup-vcpkg.sh - Setup vcpkg package manager for CLAP Host build
set -e

OS_TYPE="$1"
VCPKG_DIR="libs/vcpkg"

echo "=== Setting up vcpkg for CLAP Host build on $OS_TYPE ==="

 # サブモジュールとしてlibs/vcpkgを利用
if [ ! -d "$VCPKG_DIR" ] || [ ! -e "$VCPKG_DIR/.git" ]; then
    echo "Error: libs/vcpkg submodule not found. Please initialize submodules."
    exit 1
fi

# vcpkgサブモジュールの履歴を完全に取得
cd "$VCPKG_DIR"
if git rev-parse --is-shallow-repository >/dev/null 2>&1 && git rev-parse --is-shallow-repository | grep -q true; then
    echo "Fetching full vcpkg history (unshallow)..."
    git fetch --unshallow || git fetch --all
else
    echo "vcpkg repository is already a full clone."
fi
cd -

# Bootstrap vcpkg - 重要！
echo "Bootstrapping vcpkg..."
if [ "$OS_TYPE" = "windows" ]; then
    ./$VCPKG_DIR/bootstrap-vcpkg.bat
else
    ./$VCPKG_DIR/bootstrap-vcpkg.sh
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

echo "Installing CLAP Host dependencies via vcpkg for $TRIPLET..."
./"$VCPKG_DIR"/vcpkg install rtmidi rtaudio --triplet="$TRIPLET"

# Verify installation
echo "Verifying vcpkg package installation..."
./"$VCPKG_DIR"/vcpkg list | grep -E "(rtmidi|rtaudio)" || echo "Warning: RtMidi/RtAudio not found in vcpkg list"

# Set environment variables for CLAP Host build
export VCPKG_ROOT="$(pwd)/$VCPKG_DIR"
export CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
export VCPKG_TARGET_TRIPLET="$TRIPLET"

echo "VCPKG_ROOT=$VCPKG_ROOT" >> ${GITHUB_ENV:-/dev/null}
echo "CMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE" >> ${GITHUB_ENV:-/dev/null}
echo "VCPKG_TARGET_TRIPLET=$TRIPLET" >> ${GITHUB_ENV:-/dev/null}

# Add to PATH for current session
echo "$(pwd)/$VCPKG_DIR" >> ${GITHUB_PATH:-/dev/null}

echo "✓ vcpkg setup completed for CLAP Host"
echo "VCPKG_ROOT: $VCPKG_ROOT"
echo "CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
echo "VCPKG_TARGET_TRIPLET: $TRIPLET"

# Show installed packages for debugging
echo "Installed vcpkg packages:"
./"$VCPKG_DIR"/vcpkg list || echo "Could not list packages"

# Show include paths for debugging
echo "RtMidi include path:"
find "$VCPKG_ROOT/installed/$TRIPLET" -name "RtMidi.h" 2>/dev/null || echo "RtMidi.h not found"
