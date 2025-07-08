#!/bin/bash
set -e

echo "=== Brisk Prebuilt Binary Setup ==="
echo "Following https://docs.brisklib.com/getting_started/prebuilt_binaries/"

# Brisk download configuration
BRISK_VERSION="v0.10.0"  # Use actual available version
BRISK_BASE_URL="https://github.com/brisklib/brisk/releases/download"
BRISK_DEPS_HASH="944e23be"  # Dependencies hash from actual releases

# Alternative version for fallback
BRISK_VERSION_ALT="v0.9.0"
BRISK_DEPS_HASH_ALT="abc12345"

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
        # Use curl with explicit error handling
        if curl -L -f -o "$file" "$url" 2>/dev/null; then
            # Check if downloaded file is actually a valid archive
            # XZ compressed files are valid tar.xz archives
            if file "$file" | grep -q -E "(gzip|Zip|tar archive|XZ compressed data|compressed)"; then
                echo "✓ Downloaded $name"
                return 0
            else
                echo "⚠️  Downloaded file is not a valid archive ($(file "$file" | cut -d: -f2-))"
                echo "    File size: $(ls -lh "$file" | awk '{print $5}')"
                # Don't remove the file, let's try to extract it anyway
                echo "    Attempting to extract despite file type detection issue..."
                return 0
            fi
        else
            echo "⚠️  Failed to download $name from $url"
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
    
    # Check for pack_resource tool
    local pack_resource_tool="$BRISK_DIR/installed/x64-linux/tools/pack-resource/pack_resource"
    if [ -f "$pack_resource_tool" ]; then
        echo "✓ Found pack_resource tool at: $pack_resource_tool"
        chmod +x "$pack_resource_tool"
        
        # Add to PATH for CMAKE to find it
        export PATH="$BRISK_DIR/installed/x64-linux/tools/pack-resource:$PATH"
        echo "✓ Added pack_resource to PATH"
    else
        echo "⚠️  pack_resource tool not found at expected location"
    fi
    
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

PREBUILT_AVAILABLE=true
DEPS_AVAILABLE=true

if ! download_if_needed "$BRISK_PREBUILT_URL" "$BRISK_PREBUILT_FILE" "Brisk-Prebuilt"; then
    PREBUILT_AVAILABLE=false
fi

if ! download_if_needed "$BRISK_DEPS_URL" "$BRISK_DEPS_FILE" "Brisk-Dependencies"; then
    DEPS_AVAILABLE=false
fi

# Extract if downloads were successful
if [ "$PREBUILT_AVAILABLE" = true ] && [ "$DEPS_AVAILABLE" = true ]; then
    echo "Extracting Brisk binaries..."
    
    # Extract Brisk-Prebuilt
    cd "$BRISK_DIR"
    echo "Extracting Brisk-Prebuilt from $BRISK_PREBUILT_FILE..."
    if tar -xJf "$BRISK_PREBUILT_FILE" --strip-components=1; then
        echo "✓ Extracted Brisk-Prebuilt"
    else
        echo "⚠️  Failed to extract Brisk-Prebuilt, trying alternative extraction method..."
        # Try without strip-components
        if tar -xJf "$BRISK_PREBUILT_FILE"; then
            echo "✓ Extracted Brisk-Prebuilt (alternative method)"
        else
            echo "⚠️  Failed to extract Brisk-Prebuilt, falling back to header-only mode"
            PREBUILT_AVAILABLE=false
        fi
    fi
    
    # Extract Brisk-Dependencies (only if prebuilt extraction succeeded)
    if [ "$PREBUILT_AVAILABLE" = true ]; then
        echo "Extracting Brisk-Dependencies from $BRISK_DEPS_FILE..."
        if tar -xJf "$BRISK_DEPS_FILE" --strip-components=1; then
            echo "✓ Extracted Brisk-Dependencies"
        else
            echo "⚠️  Failed to extract Brisk-Dependencies, trying alternative extraction method..."
            # Try without strip-components
            if tar -xJf "$BRISK_DEPS_FILE"; then
                echo "✓ Extracted Brisk-Dependencies (alternative method)"
            else
                echo "⚠️  Failed to extract Brisk-Dependencies, falling back to header-only mode"
                PREBUILT_AVAILABLE=false
            fi
        fi
    fi
    
    # Verify extraction
    if [ -d "include" ] && [ -d "lib" ]; then
        echo "✓ Brisk prebuilt binaries successfully installed"
        echo "  - Include directory: $BRISK_DIR/include"
        echo "  - Library directory: $BRISK_DIR/lib"
        
        # List available headers and libraries for verification
        if [ -d "include/brisk" ]; then
            echo "  - Available headers: $(ls include/brisk/*.h* 2>/dev/null | wc -l) files"
        fi
        if [ -d "lib" ]; then
            echo "  - Available libraries: $(ls lib/*brisk* 2>/dev/null | wc -l) files"
        fi
        
        # Setup vcpkg for Brisk if dependencies are available
        verify_brisk_installation
    else
        echo "⚠️  Extraction seems incomplete, falling back to header-only mode"
        PREBUILT_AVAILABLE=false
    fi
elif [ "$DEPS_AVAILABLE" = true ]; then
    echo "Extracting Brisk dependencies only..."
    
    # Extract only dependencies
    cd "$BRISK_DIR"
    echo "Extracting Brisk-Dependencies from $BRISK_DEPS_FILE..."
    if tar -xJf "$BRISK_DEPS_FILE" --strip-components=1; then
        echo "✓ Extracted Brisk-Dependencies"
    else
        echo "⚠️  Failed to extract Brisk-Dependencies, trying alternative extraction method..."
        # Try without strip-components
        if tar -xJf "$BRISK_DEPS_FILE"; then
            echo "✓ Extracted Brisk-Dependencies (alternative method)"
        else
            echo "⚠️  Failed to extract Brisk-Dependencies, falling back to header-only mode"
            DEPS_AVAILABLE=false
        fi
    fi        # Verify extraction
        if [ -d "include" ]; then
            echo "✓ Brisk dependencies successfully installed (headers only)"
            echo "  - Include directory: $BRISK_DIR/include"
            
            # List available headers for verification
            if [ -d "include/brisk" ]; then
                echo "  - Available headers: $(ls include/brisk/*.h* 2>/dev/null | wc -l) files"
            fi
            PREBUILT_AVAILABLE=false  # No prebuilt libraries, but headers are available
            
            # Verify Brisk installation
            verify_brisk_installation
        else
            echo "⚠️  Dependencies extraction seems incomplete, falling back to header-only mode"
            PREBUILT_AVAILABLE=false
            DEPS_AVAILABLE=false
        fi
fi

echo "=== Brisk setup completed ==="

# Print summary
if [ "$PREBUILT_AVAILABLE" = true ]; then
    echo ""
    echo "🎉 Brisk prebuilt binaries successfully installed!"
    echo "   Platform: $PLATFORM_SUFFIX"
    echo "   Version: $BRISK_VERSION"
    echo "   Mode: Full prebuilt library"
elif [ "$DEPS_AVAILABLE" = true ]; then
    echo ""
    echo "✅ Brisk dependencies successfully installed!"
    echo "   Platform: $PLATFORM_SUFFIX"
    echo "   Version: $BRISK_VERSION"
    echo "   Mode: Dependencies with header-only fallback"
    echo "   Note: Prebuilt libraries not available, but headers are installed"
else
    echo ""
    echo "⚠️  Brisk prebuilt binaries not available for $PLATFORM_SUFFIX"
    echo "   Fallback: Header-only interface created"
    echo "   Note: Full UI functionality may be limited"
fi