#!/bin/bash
# setup-dependencies.sh - Setup build dependencies for different platforms
set -e

OS_TYPE="$1"

echo "=== Setting up dependencies for $OS_TYPE ==="

case "$OS_TYPE" in
    "linux")
        echo "Installing GUI dependencies for Linux..."
        sudo apt-get update -y
        
    for attempt in 1 2 3; do
            if sudo apt-get install -y --no-install-recommends \
                libx11-dev \
                libxcursor-dev \
                libxrandr-dev \
                libxinerama-dev \
                libxi-dev \
                libgl1-mesa-dev \
                libglu1-mesa-dev \
                libxxf86vm-dev \
                libxfixes-dev \
        libvulkan1 \
        mesa-vulkan-drivers \
        vulkan-tools \
                pkg-config \
                build-essential \
                mesa-utils \
                ninja-build \
                cmake \
                git \
                curl \
                zip \
                unzip \
                tar \
                xvfb \
                x11-utils \
                imagemagick \
                scrot; then
                echo "Dependencies installed successfully"
                break
            else
                echo "Attempt $attempt failed, retrying..."
                sleep 2
            fi
        done
        ;;
        
    "macos")
        echo "Installing essential build tools on macOS..."
        brew install pkg-config ninja cmake git curl
        ;;
        
    "windows")
        echo "Windows dependencies will be handled by MSVC setup and build scripts"
        ;;
        
    *)
        echo "Unknown OS type: $OS_TYPE"
        exit 1
        ;;
esac

echo "✓ Dependencies setup completed for $OS_TYPE"
