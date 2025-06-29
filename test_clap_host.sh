#!/bin/bash

# Simple test script to verify CLAP host integration locally
# This script can be used for local development and testing

set -e

echo "=== CLAP Host Integration Test Script ==="

# Build the plugin first
echo "Building plugin..."
cmake . -B build
cmake --build build --config Release

# Check if plugin was built
if [ "$OSTYPE" = "linux-gnu"* ]; then
    PLUGIN_FILE="build/MyFirstClapPlugin.so"
elif [[ "$OSTYPE" = "darwin"* ]]; then
    PLUGIN_FILE="build/MyFirstClapPlugin.dylib"
elif [[ "$OSTYPE" = "msys" || "$OSTYPE" = "cygwin" ]]; then
    PLUGIN_FILE="build/Release/MyFirstClapPlugin.clap"
else
    echo "Unsupported OS type: $OSTYPE"
    exit 1
fi

if [ ! -f "$PLUGIN_FILE" ]; then
    echo "Error: Plugin file not found at $PLUGIN_FILE"
    exit 1
fi

echo "Plugin built successfully: $PLUGIN_FILE"

# Check if clap-host-repo exists
if [ ! -d "clap-host-repo" ]; then
    echo "Cloning clap-host repository..."
    git clone --recurse-submodules https://github.com/free-audio/clap-host.git clap-host-repo
fi

cd clap-host-repo

# Build clap-host
echo "Building clap-host..."
if cmake --preset ninja-system 2>/dev/null; then
    echo "Using ninja-system preset"
    cmake --build --preset ninja-system
else
    echo "Using manual configuration"
    cmake . -B builds/manual -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build builds/manual --config Release
fi

# Find clap-host executable
CLAP_HOST_EXEC=$(find builds -name "clap-host" -type f | head -1)
if [ -z "$CLAP_HOST_EXEC" ]; then
    echo "Error: clap-host executable not found!"
    exit 1
fi

echo "Found clap-host at: $CLAP_HOST_EXEC"
chmod +x "$CLAP_HOST_EXEC"

# Test the plugin with clap-host
echo "Testing plugin with clap-host..."
echo "Plugin file: $(pwd)/../$PLUGIN_FILE"

# Show help
echo "=== clap-host help ==="
"$CLAP_HOST_EXEC" --help

echo ""
echo "=== Testing plugin loading ==="
echo "Running: $CLAP_HOST_EXEC -p $(pwd)/../$PLUGIN_FILE"
echo "Note: This will open a GUI window. Close it manually to continue."

"$CLAP_HOST_EXEC" -p "$(pwd)/../$PLUGIN_FILE"

echo "Test completed!"