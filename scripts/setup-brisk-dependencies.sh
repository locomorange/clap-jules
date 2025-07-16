#!/bin/bash
set -e

# Setup Brisk-specific dependencies
# This script installs additional dependencies required for Brisk library

echo "Setting up Brisk dependencies..."

# Detect OS and install dependencies accordingly
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    echo "Installing Brisk dependencies for macOS..."
    brew install cmake git ninja autoconf automake autoconf-archive
    echo "✓ macOS Brisk dependencies installed"
    
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    echo "Installing Brisk dependencies for Linux..."
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
    echo "✓ Linux Brisk dependencies installed"
    
elif [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "cygwin"* || "$OSTYPE" == "win32" ]]; then
    # Windows (Git Bash/MSYS environment)
    echo "Windows environment detected. Dependencies will be handled by vcpkg."
    # On Windows, we rely on vcpkg for dependency management
    # CMake, Ninja, and other build tools are expected to be available
    echo "✓ Windows Brisk dependencies (handled by vcpkg)"
    
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

echo "✅ Brisk dependencies setup completed"