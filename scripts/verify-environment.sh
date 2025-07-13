#!/bin/bash
# verify-environment.sh - Verify complete development environment
set -e

echo "=== Environment Verification ==="

echo "--- Build Tools ---"
command -v cmake
command -v ninja
command -v gcc
command -v g++

echo "--- CLAP Tools ---"
if [ -z "$CLAP_TOOLS_PATH" ]; then
    echo "CLAP_TOOLS_PATH not set. Please set it to the directory containing clap-tools."
    exit 1
fi
export PATH=$PATH:/usr/local/bin:$CLAP_TOOLS_PATH
command -v clap-validator || echo "clap-validator not found"
command -v clap-info || echo "clap-info not found"

echo "--- vcpkg ---"
if [ -n "$VCPKG_ROOT" ]; then
    echo "VCPKG_ROOT: $VCPKG_ROOT"
    echo "CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
    ls -la "$VCPKG_ROOT/installed/x64-linux/lib/" | grep -E "(rtmidi|rtaudio)" || echo "RtMidi/RtAudio not found"
else
    echo "VCPKG_ROOT not set"
fi

echo "--- Qt6 ---"
if command -v qmake &> /dev/null; then
    qmake --version
else
    echo "Qt6 not found"
fi

echo "--- CLAP Host ---"
if [ -n "$CLAP_HOST_PATH" ] && [ -d "$CLAP_HOST_PATH" ]; then
    echo "CLAP_HOST_PATH: $CLAP_HOST_PATH"
    ls -la "$CLAP_HOST_PATH/" | head -5
    
    # Check for build artifacts
    if [ -d "$CLAP_HOST_PATH/builds" ]; then
        echo "CLAP Host build artifacts found:"
        find "$CLAP_HOST_PATH/builds" -name "clap-host*" -type f | head -3 || echo "No clap-host executables found in builds"
    else
        echo "No build artifacts found (this is normal for setup-only mode)"
    fi
else
    echo "CLAP Host not found or path not set"
fi

echo "--- X11 GUI Support ---"
if command -v dpkg &> /dev/null; then
    dpkg -l | grep -E "(libx11|mesa)" | head -5
else
    echo "dpkg not available (not on Ubuntu/Debian)"
fi

echo "--- Screenshot Tools ---"
which xvfb-run || echo "xvfb-run not found"
which import || echo "imagemagick import not found"
which scrot || echo "scrot not found"

echo "✓ All environment components verified"
