#!/bin/bash
set -e

echo "Installing Brisk dependencies..."

# Detect OS and install dependencies accordingly
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    brew install cmake git ninja autoconf automake autoconf-archive
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    sudo apt-get update
    sudo apt-get install -y \
        ninja-build \
        mesa-vulkan-drivers \
        vulkan-tools \
        wget \
        xorg-dev \
        libgl-dev \
        libgl1-mesa-dev \
        libvulkan-dev \
        autoconf \
        autoconf-archive \
        libxrandr-dev \
        libxinerama-dev \
        libxcursor-dev \
        mesa-common-dev \
        libx11-xcb-dev \
        libwayland-dev \
        libxkbcommon-dev
elif [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "cygwin"* || "$OSTYPE" == "win32" ]]; then
    # Windows (Git Bash/MSYS environment)
    echo "Windows environment detected. Dependencies will be handled by vcpkg."
    # On Windows, we rely on vcpkg for dependency management
    # CMake, Ninja, and other build tools are expected to be available
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

# OSごとにtripletを自動選択
if [[ "$OSTYPE" == "darwin"* ]]; then
    if [[ $(uname -m) == "arm64" ]]; then
        BRISK_TRIPLET="arm64-osx"
    else
        BRISK_TRIPLET="x64-osx"
    fi
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    BRISK_TRIPLET="x64-linux"
elif [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "cygwin"* || "$OSTYPE" == "win32" ]]; then
    BRISK_TRIPLET="x64-windows-static"
else
    echo "Unsupported OS for triplet: $OSTYPE"
    exit 1
fi

echo "Building Brisk..."
cd libs/brisk

cmake -G Ninja \
    -S . \
    -DVCPKG_MANIFEST_INSTALL=ON \
    -DCMAKE_INSTALL_PREFIX=dist \
    -DVCPKG_TARGET_TRIPLET=${BRISK_TRIPLET} \
    -DCMAKE_TOOLCHAIN_FILE=../../vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_INSTALLED_DIR=../../vcpkg/installed \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON

#    -B build-release \

cmake --build build-release --target install

echo "Brisk build completed successfully!"