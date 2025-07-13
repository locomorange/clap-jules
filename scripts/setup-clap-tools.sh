#!/bin/bash
# setup-clap-tools.sh - Download and setup CLAP validation tools
set -e

OS_TYPE="$1"
CLAP_VALIDATOR_VERSION="${2:-0.3.2}"
CLAP_INFO_VERSION="${3:-v1.2.2}"

echo "=== Setting up CLAP tools for $OS_TYPE ==="
echo "CLAP Validator version: $CLAP_VALIDATOR_VERSION"
echo "CLAP Info version: $CLAP_INFO_VERSION"

case "$OS_TYPE" in
    "linux")
        # Download clap-validator
        wget -O clap-validator.tar.gz "https://github.com/free-audio/clap-validator/releases/download/${CLAP_VALIDATOR_VERSION}/clap-validator-${CLAP_VALIDATOR_VERSION}-ubuntu-18.04.tar.gz" || echo "clap-validator download failed"
        if [ -f clap-validator.tar.gz ]; then
            tar -xzf clap-validator.tar.gz && chmod +x clap-validator || echo "clap-validator extraction failed"
        fi
        
        # Download clap-info
        wget -O clap-info.zip "https://github.com/free-audio/clap-info/releases/download/${CLAP_INFO_VERSION}/clap-info-linux-x64.zip" || echo "clap-info download failed"
        if [ -f clap-info.zip ]; then
            unzip -o -q clap-info.zip && chmod +x clap-info || echo "clap-info extraction failed"
        fi
        ;;
        
    "windows")
        # Download clap-validator
        curl -L -o clap-validator.zip "https://github.com/free-audio/clap-validator/releases/download/${CLAP_VALIDATOR_VERSION}/clap-validator-${CLAP_VALIDATOR_VERSION}-windows.zip" || echo "clap-validator download failed"
        if [ -f clap-validator.zip ]; then
            unzip -o -q clap-validator.zip || echo "clap-validator extraction failed"
        fi
        
        # Download clap-info
        curl -L -o clap-info.zip "https://github.com/free-audio/clap-info/releases/download/${CLAP_INFO_VERSION}/clap-info-win-x64.zip" || echo "clap-info download failed"
        if [ -f clap-info.zip ]; then
            unzip -o -q clap-info.zip || echo "clap-info extraction failed"
        fi
        ;;
        
    "macos")
        # Download clap-validator
        curl -L -o clap-validator.zip "https://github.com/free-audio/clap-validator/releases/download/${CLAP_VALIDATOR_VERSION}/clap-validator-macos.zip" || echo "clap-validator download failed"
        if [ -f clap-validator.zip ]; then
            unzip -o -q clap-validator.zip && chmod +x clap-validator || echo "clap-validator extraction failed"
        fi
        
        # Download clap-info
        curl -L -o clap-info.zip "https://github.com/free-audio/clap-info/releases/download/${CLAP_INFO_VERSION}/clap-info-mac-universal.zip" || echo "clap-info download failed"
        if [ -f clap-info.zip ]; then
            unzip -o -q clap-info.zip && chmod +x clap-info || echo "clap-info extraction failed"
        fi
        ;;
        
    *)
        echo "Unknown OS type: $OS_TYPE"
        exit 1
        ;;
esac

echo "✓ CLAP tools setup completed for $OS_TYPE"
