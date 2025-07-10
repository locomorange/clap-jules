#!/bin/bash
# prepare-clap-host.sh - Prepare CLAP Host repository for development
set -e

CLAP_HOST_DIR="${1:-clap-host-repo}"

echo "=== Preparing CLAP Host repository ==="

# CLAP Hostクローン
if [ ! -d "$CLAP_HOST_DIR" ] || [ ! -f "$CLAP_HOST_DIR/.git/config" ]; then
    echo "Cloning CLAP Host repository..."
    rm -rf "$CLAP_HOST_DIR" 2>/dev/null || true
    git clone --recurse-submodules https://github.com/free-audio/clap-host.git "$CLAP_HOST_DIR"
else
    echo "CLAP Host repository already exists, updating..."
    cd "$CLAP_HOST_DIR"
    git pull --recurse-submodules || echo "Git pull failed, continuing with existing version"
    cd ..
fi

# 環境変数設定
export CLAP_HOST_PATH="$(pwd)/$CLAP_HOST_DIR"
echo "CLAP_HOST_PATH=$CLAP_HOST_PATH" >> ${GITHUB_ENV:-/dev/null}

echo "✓ CLAP Host repository prepared at: $CLAP_HOST_PATH"
