#!/bin/bash
set -e

echo "=== Brisk Prebuilt Binary Setup ==="
echo "Following https://docs.brisklib.com/getting_started/prebuilt_binaries/"

# Brisk download configuration
BRISK_VERSION="v1.0"  # Use a stable version instead of latest
BRISK_BASE_URL="https://github.com/brisklib/brisk/releases/download"

# Detect platform and architecture
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if [[ $(uname -m) == "x86_64" ]]; then
        PLATFORM="Linux-x86_64"
    else
        echo "Unsupported Linux architecture: $(uname -m)"
        exit 1
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    if [[ $(uname -m) == "x86_64" ]]; then
        PLATFORM="macOS-x86_64"
    elif [[ $(uname -m) == "arm64" ]]; then
        PLATFORM="macOS-arm64"
    else
        echo "Unsupported macOS architecture: $(uname -m)"
        exit 1
    fi
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    PLATFORM="Windows-x86_64"
else
    echo "Unsupported platform: $OSTYPE"
    exit 1
fi

BRISK_DIR="${CMAKE_CURRENT_SOURCE_DIR:-$(pwd)}/libs/brisk"
BRISK_CACHE_DIR="${BRISK_DIR}/.cache"

echo "Platform: $PLATFORM"
echo "Brisk directory: $BRISK_DIR"

# Create directories
mkdir -p "$BRISK_DIR"
mkdir -p "$BRISK_CACHE_DIR"

# Download URLs according to Brisk prebuilt binary naming convention
BRISK_PREBUILT_URL="${BRISK_BASE_URL}/${BRISK_VERSION}/Brisk-Prebuilt-${PLATFORM}.tar.gz"
BRISK_DEPS_URL="${BRISK_BASE_URL}/${BRISK_VERSION}/Brisk-Dependencies-${PLATFORM}.tar.gz"

BRISK_PREBUILT_FILE="${BRISK_CACHE_DIR}/Brisk-Prebuilt-${PLATFORM}.tar.gz"
BRISK_DEPS_FILE="${BRISK_CACHE_DIR}/Brisk-Dependencies-${PLATFORM}.tar.gz"

# Function to download and cache files
download_if_needed() {
    local url="$1"
    local file="$2"
    local name="$3"
    
    if [ ! -f "$file" ]; then
        echo "Downloading $name from $url..."
        # Use curl with explicit error handling and check for valid content
        if curl -L -f -o "$file" "$url" 2>/dev/null; then
            # Check if downloaded file is actually a valid archive
            if file "$file" | grep -q -E "(gzip|Zip|tar archive)"; then
                echo "✓ Downloaded $name"
                return 0
            else
                echo "⚠️  Downloaded file is not a valid archive ($(file "$file" | cut -d: -f2-))"
                rm -f "$file"
                return 1
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

# Download prebuilt binaries
echo "Downloading Brisk prebuilt binaries for $PLATFORM..."

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
    tar -xzf "$BRISK_PREBUILT_FILE" --strip-components=1
    echo "✓ Extracted Brisk-Prebuilt"
    
    # Extract Brisk-Dependencies
    tar -xzf "$BRISK_DEPS_FILE" --strip-components=1
    echo "✓ Extracted Brisk-Dependencies"
    
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
    else
        echo "⚠️  Extraction seems incomplete, falling back to header-only mode"
        PREBUILT_AVAILABLE=false
    fi
fi

# Fallback: Create minimal header-only interface
if [ "$PREBUILT_AVAILABLE" = false ]; then
    echo "Creating minimal header-only Brisk interface..."
    
    mkdir -p "$BRISK_DIR/include/brisk"
    
    cat > "$BRISK_DIR/include/brisk/brisk.h" << 'EOF'
#pragma once

// Minimal Brisk header-only interface
// This is a fallback when prebuilt binaries are not available

#include <functional>
#include <memory>

namespace brisk {

// Forward declarations
class Window;
class Widget;

// Basic application lifecycle
class Application {
public:
    static void Initialize();
    static void Shutdown();
    static bool IsInitialized();
    
private:
    static bool initialized_;
};

// Basic window class
class Window {
public:
    explicit Window(void* parent_handle = nullptr);
    virtual ~Window();
    
    void SetVisible(bool visible);
    void SetSize(int width, int height);
    void* GetHandle() const;
    void Update();
    
    // Widget management
    void AddWidget(std::shared_ptr<Widget> widget);
    void RemoveWidget(std::shared_ptr<Widget> widget);
    
private:
    void* parent_handle_;
    bool visible_;
    int width_, height_;
    std::vector<std::shared_ptr<Widget>> widgets_;
};

// Base widget class
class Widget {
public:
    virtual ~Widget() = default;
    virtual void Draw() = 0;
    virtual void Update() {}
    
    void SetPosition(int x, int y) { x_ = x; y_ = y; }
    void SetSize(int width, int height) { width_ = width; height_ = height; }
    
protected:
    int x_ = 0, y_ = 0;
    int width_ = 100, height_ = 50;
};

// Knob widget for parameter control
class Knob : public Widget {
public:
    Knob(double min_val, double max_val, double initial_val);
    
    void SetValue(double value);
    double GetValue() const;
    void SetCallback(std::function<void(double)> callback);
    
    void Draw() override;
    
private:
    double min_value_, max_value_, current_value_;
    std::function<void(double)> callback_;
};

} // namespace brisk
EOF

    echo "✓ Minimal Brisk header-only interface created"
fi

# Create CMake configuration
cat > "$BRISK_DIR/BriskConfig.cmake" << EOF
# Brisk CMake configuration
# Generated by setup-brisk.sh

set(BRISK_FOUND TRUE)
set(BRISK_INCLUDE_DIR "\${CMAKE_CURRENT_LIST_DIR}/include")
set(BRISK_LIBRARY_DIR "\${CMAKE_CURRENT_LIST_DIR}/lib")

# Create brisk target
if(NOT TARGET brisk)
    if(EXISTS "\${BRISK_LIBRARY_DIR}")
        # Full prebuilt library mode
        find_library(BRISK_LIBRARY 
            NAMES brisk libbrisk
            PATHS "\${BRISK_LIBRARY_DIR}"
            NO_DEFAULT_PATH
        )
        
        if(BRISK_LIBRARY)
            add_library(brisk SHARED IMPORTED)
            set_target_properties(brisk PROPERTIES
                IMPORTED_LOCATION "\${BRISK_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "\${BRISK_INCLUDE_DIR}"
            )
            message(STATUS "Brisk: Using prebuilt shared library at \${BRISK_LIBRARY}")
        else()
            # Header-only fallback - check if we have implementation
            if(EXISTS "\${CMAKE_CURRENT_LIST_DIR}/src/brisk.cpp")
                # Create static library with implementation
                add_library(brisk STATIC "\${CMAKE_CURRENT_LIST_DIR}/src/brisk.cpp")
                target_include_directories(brisk PUBLIC "\${BRISK_INCLUDE_DIR}")
                set_target_properties(brisk PROPERTIES POSITION_INDEPENDENT_CODE ON)
                message(STATUS "Brisk: Using static library with header-only implementation")
            else()
                # Pure header-only mode
                add_library(brisk INTERFACE)
                target_include_directories(brisk INTERFACE "\${BRISK_INCLUDE_DIR}")
                message(STATUS "Brisk: Using header-only interface (no prebuilt library found)")
            endif()
        endif()
    else()
        # Header-only mode - check if we have implementation
        if(EXISTS "\${CMAKE_CURRENT_LIST_DIR}/src/brisk.cpp")
            # Create static library with implementation
            add_library(brisk STATIC "\${CMAKE_CURRENT_LIST_DIR}/src/brisk.cpp")
            target_include_directories(brisk PUBLIC "\${BRISK_INCLUDE_DIR}")
            set_target_properties(brisk PROPERTIES POSITION_INDEPENDENT_CODE ON)
            message(STATUS "Brisk: Using static library with header-only implementation")
        else()
            # Pure header-only mode
            add_library(brisk INTERFACE)
            target_include_directories(brisk INTERFACE "\${BRISK_INCLUDE_DIR}")
            message(STATUS "Brisk: Using header-only interface")
        endif()
    endif()
endif()

message(STATUS "Brisk found: \${BRISK_INCLUDE_DIR}")
EOF

echo "✓ Brisk CMake configuration created"
echo "=== Brisk setup completed ==="

# Print summary
if [ "$PREBUILT_AVAILABLE" = true ]; then
    echo ""
    echo "🎉 Brisk prebuilt binaries successfully installed!"
    echo "   Platform: $PLATFORM"
    echo "   Version: $BRISK_VERSION"
    echo "   Mode: Full prebuilt library"
else
    echo ""
    echo "⚠️  Brisk prebuilt binaries not available for $PLATFORM"
    echo "   Fallback: Header-only interface created"
    echo "   Note: Full UI functionality may be limited"
fi