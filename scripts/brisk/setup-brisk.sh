#!/bin/bash
set -e

echo "=== Brisk Prebuilt Binary Setup ==="
echo "Following https://docs.brisklib.com/getting_started/prebuilt_binaries/"

# Brisk download configuration
BRISK_VERSION="v0.10.0"  # Use actual available version
BRISK_BASE_URL="https://github.com/brisklib/brisk/releases/download"
BRISK_DEPS_HASH="944e23be"  # Dependencies hash from actual releases

# Detect platform and architecture
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if [[ $(uname -m) == "x86_64" ]]; then
        PLATFORM_SUFFIX="x64-linux"
    else
        echo "Unsupported Linux architecture: $(uname -m)"
        exit 1
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS uses universal binary
    PLATFORM_SUFFIX="uni-osx"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    PLATFORM_SUFFIX="x64-windows-static-md"
else
    echo "Unsupported platform: $OSTYPE"
    exit 1
fi

BRISK_DIR="${CMAKE_CURRENT_SOURCE_DIR:-$(pwd)}/libs/brisk"
BRISK_CACHE_DIR="${BRISK_DIR}/.cache"

echo "Platform: $PLATFORM_SUFFIX"
echo "Brisk directory: $BRISK_DIR"

# Create directories
mkdir -p "$BRISK_DIR"
mkdir -p "$BRISK_CACHE_DIR"

# Download URLs according to Brisk prebuilt binary naming convention
BRISK_PREBUILT_URL="${BRISK_BASE_URL}/${BRISK_VERSION}/Brisk-Prebuilt-${BRISK_VERSION}-${PLATFORM_SUFFIX}.tar.xz"
BRISK_DEPS_URL="${BRISK_BASE_URL}/${BRISK_VERSION}/Brisk-Dependencies-${BRISK_DEPS_HASH}-${PLATFORM_SUFFIX}.tar.xz"

BRISK_PREBUILT_FILE="${BRISK_CACHE_DIR}/Brisk-Prebuilt-${BRISK_VERSION}-${PLATFORM_SUFFIX}.tar.xz"
BRISK_DEPS_FILE="${BRISK_CACHE_DIR}/Brisk-Dependencies-${BRISK_DEPS_HASH}-${PLATFORM_SUFFIX}.tar.xz"

# Function to download and cache files
download_if_needed() {
    local url="$1"
    local file="$2"
    local name="$3"
    
    if [ ! -f "$file" ]; then
        echo "Downloading $name from $url..."
        if curl -L -f -o "$file" "$url"; then
            if file "$file" | grep -q -E "(gzip|Zip|tar archive|XZ compressed data|compressed)"; then
                echo "✓ Downloaded $name"
                return 0
            else
                echo "❌ Downloaded file is not a valid archive"
                rm -f "$file"
                return 1
            fi
        else
            echo "❌ Failed to download $name from $url"
            return 1
        fi
    else
        echo "✓ Using cached $name"
        return 0
    fi
}

# Function to verify Brisk installation
verify_brisk_installation() {
    echo "=== Verifying Brisk installation ==="
    
    # Check for other essential tools
    local tools_dir="$BRISK_DIR/installed/x64-linux/tools"
    if [ -d "$tools_dir" ]; then
        echo "✓ Found tools directory with $(ls "$tools_dir" | wc -l) tool packages"
    fi
    
    # Check for headers
    if [ -d "$BRISK_DIR/include/brisk" ]; then
        echo "✓ Found Brisk headers"
    fi
    
    # Check for libraries
    if [ -d "$BRISK_DIR/lib" ] && [ -n "$(ls "$BRISK_DIR/lib"/*brisk* 2>/dev/null)" ]; then
        echo "✓ Found Brisk libraries"
    fi
}

# Download prebuilt binaries
echo "Downloading Brisk prebuilt binaries for $PLATFORM_SUFFIX..."

if ! download_if_needed "$BRISK_PREBUILT_URL" "$BRISK_PREBUILT_FILE" "Brisk-Prebuilt"; then
    echo "❌ Failed to download Brisk prebuilt binaries"
    exit 1
fi

if ! download_if_needed "$BRISK_DEPS_URL" "$BRISK_DEPS_FILE" "Brisk-Dependencies"; then
    echo "❌ Failed to download Brisk dependencies"
    exit 1
fi

# Extract binaries
echo "Extracting Brisk binaries..."

cd "$BRISK_DIR"
echo "Extracting Brisk-Prebuilt from $BRISK_PREBUILT_FILE..."
if ! tar -xJf "$BRISK_PREBUILT_FILE" --strip-components=1; then
    echo "❌ Failed to extract Brisk-Prebuilt"
    exit 1
fi
echo "✓ Extracted Brisk-Prebuilt"

echo "Extracting Brisk-Dependencies from $BRISK_DEPS_FILE..."
if ! tar -xJf "$BRISK_DEPS_FILE" --strip-components=1; then
    echo "❌ Failed to extract Brisk-Dependencies"
    exit 1
fi
echo "✓ Extracted Brisk-Dependencies"

# Verify extraction
if [ ! -d "include" ] || [ ! -d "lib" ]; then
    echo "❌ Brisk installation verification failed: missing include or lib directories"
    exit 1
fi

echo "✓ Brisk prebuilt binaries successfully installed"
echo "  - Include directory: $BRISK_DIR/include"
echo "  - Library directory: $BRISK_DIR/lib"

# List available headers and libraries for verification
if [ -d "include/brisk" ]; then
    echo "  - Available headers: $(find include/brisk -name "*.h*" | wc -l) files"
fi
if [ -d "lib" ]; then
    echo "  - Available libraries: $(ls lib/*brisk* 2>/dev/null | wc -l) files"
fi

verify_brisk_installation

echo "=== Brisk setup completed ==="

echo ""
echo "🎉 Brisk prebuilt binaries successfully installed!"
echo "   Platform: $PLATFORM_SUFFIX"
echo "   Version: $BRISK_VERSION"
echo "   Mode: Full prebuilt library"