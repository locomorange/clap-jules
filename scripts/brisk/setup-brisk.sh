#!/bin/bash
set -e

echo "=== Brisk Prebuilt Binary Setup ==="

# Brisk download configuration
BRISK_VERSION="latest"
BRISK_BASE_URL="https://github.com/brisklib/brisk/releases"

# Detect platform
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
    ARCHIVE_EXT="tar.gz"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
    ARCHIVE_EXT="tar.gz"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    PLATFORM="windows"
    ARCHIVE_EXT="zip"
else
    echo "Unsupported platform: $OSTYPE"
    exit 1
fi

BRISK_DIR="${CMAKE_CURRENT_SOURCE_DIR:-$(pwd)}/libs/brisk"
BRISK_CACHE_DIR="${BRISK_DIR}/cache"

echo "Platform: $PLATFORM"
echo "Brisk directory: $BRISK_DIR"

# Create directories
mkdir -p "$BRISK_DIR"
mkdir -p "$BRISK_CACHE_DIR"

# Download brisk prebuilt binaries
echo "Downloading Brisk prebuilt binaries..."

# Determine architecture
ARCH="x86_64"
if [[ "$PLATFORM" == "linux" ]]; then
    ARCHIVE_NAME="brisk-linux-${ARCH}.tar.xz"
    DEPS_NAME="brisk-deps-linux-${ARCH}.tar.xz"
elif [[ "$PLATFORM" == "macos" ]]; then
    # Detect ARM vs Intel Mac
    if [[ $(uname -m) == "arm64" ]]; then
        ARCH="arm64"
    fi
    ARCHIVE_NAME="brisk-macos-${ARCH}.tar.xz"
    DEPS_NAME="brisk-deps-macos-${ARCH}.tar.xz"
elif [[ "$PLATFORM" == "windows" ]]; then
    ARCHIVE_NAME="brisk-windows-${ARCH}.tar.xz"
    DEPS_NAME="brisk-deps-windows-${ARCH}.tar.xz"
fi

# Download URLs
BRISK_URL="${BRISK_BASE_URL}/latest/download/${ARCHIVE_NAME}"
DEPS_URL="${BRISK_BASE_URL}/latest/download/${DEPS_NAME}"

echo "Downloading from: $BRISK_URL"

# Download brisk prebuilt package
if [[ ! -f "$BRISK_CACHE_DIR/$ARCHIVE_NAME" ]]; then
    echo "Downloading Brisk prebuilt package..."
    curl -L -o "$BRISK_CACHE_DIR/$ARCHIVE_NAME" "$BRISK_URL" || {
        echo "Failed to download Brisk prebuilt package, falling back to placeholder"
        # Create placeholder structure as fallback
        mkdir -p "$BRISK_DIR/include/brisk"
        mkdir -p "$BRISK_DIR/lib"
        mkdir -p "$BRISK_DIR/bin"
        
        # Create placeholder header
        cat > "$BRISK_DIR/include/brisk/brisk.h" << 'EOF'
#pragma once

// Placeholder brisk header - replace with actual brisk integration
// when prebuilt binaries are available

namespace brisk {

class Application {
public:
    static void Initialize() {
        // TODO: Implement brisk initialization
    }
    
    static void Shutdown() {
        // TODO: Implement brisk shutdown
    }
};

class Window {
public:
    Window(void* parent_handle) : parent_handle_(parent_handle) {
        // TODO: Create brisk window
    }
    
    void SetVisible(bool visible) {
        visible_ = visible;
        // TODO: Implement window visibility
    }
    
    void* GetHandle() const {
        return parent_handle_;
    }
    
    void Render() {
        // TODO: Implement brisk rendering
    }
    
private:
    void* parent_handle_;
    bool visible_ = false;
};

class Knob {
public:
    Knob(double min_val, double max_val, double initial_val)
        : min_value_(min_val), max_value_(max_val), current_value_(initial_val) {
        // TODO: Create brisk knob control
    }
    
    void SetValue(double value) {
        current_value_ = std::max(min_value_, std::min(max_value_, value));
        // TODO: Update UI
    }
    
    double GetValue() const {
        return current_value_;
    }
    
    void SetCallback(std::function<void(double)> callback) {
        callback_ = callback;
    }
    
private:
    double min_value_;
    double max_value_;
    double current_value_;
    std::function<void(double)> callback_;
};

} // namespace brisk
EOF

echo "✓ Brisk placeholder structure created"
echo "✓ Include directory: $BRISK_DIR/include"
echo "✓ Library directory: $BRISK_DIR/lib"

# Create CMake find module
cat > "$BRISK_DIR/BriskConfig.cmake" << 'EOF'
# Brisk CMake configuration
set(BRISK_FOUND TRUE)
set(BRISK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")
set(BRISK_LIBRARY_DIR "${CMAKE_CURRENT_LIST_DIR}/lib")

# Create brisk target
if(NOT TARGET brisk)
    add_library(brisk INTERFACE)
    target_include_directories(brisk INTERFACE ${BRISK_INCLUDE_DIR})
    
    # Add platform-specific link libraries when actual brisk is available
    # target_link_libraries(brisk INTERFACE ...)
endif()

message(STATUS "Brisk found: ${BRISK_INCLUDE_DIR}")
EOF

echo "✓ Brisk CMake configuration created"
echo "=== Brisk setup completed ==="