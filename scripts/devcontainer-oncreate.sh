#!/bin/bash
set -e

# 環境変数の設定
export CLAP_VALIDATOR_VERSION=0.3.2
export CLAP_INFO_VERSION=v1.2.2
export QT_BASE=/workspaces/clap-jules/tools/qt
export CLAP_TOOLS_BASE=/workspaces/clap-jules/tools/clap-tools
export VCPKG_BASE=libs/vcpkg
export CLAP_HOST_BASE=/workspaces/clap-jules/tools/clap-host
export Qt6_DIR=$QT_BASE/6.5.0/gcc_64
export QT_PLUGIN_PATH=$QT_BASE/6.5.0/gcc_64/plugins
export QML2_IMPORT_PATH=$QT_BASE/6.5.0/gcc_64/qml
export VCPKG_ROOT=$VCPKG_BASE
export PATH=$QT_BASE/6.5.0/gcc_64/bin:$VCPKG_BASE:$CLAP_TOOLS_BASE:$PATH
export CLAP_HOST_TEMPLATE_PATH=$CLAP_HOST_BASE
export CLAP_HOST_PATH=/workspaces/clap-jules/tools/clap-host/builds
export CMAKE_TOOLCHAIN_FILE=$VCPKG_BASE/scripts/buildsystems/vcpkg.cmake

# 必要なパッケージのインストール
sudo apt-get update && sudo apt-get install -y sudo curl wget make python3 python3-pip && sudo rm -rf /var/lib/apt/lists/*

# スクリプトに実行権限付与
chmod +x ./scripts/*.sh

# 依存関係セットアップ
./scripts/setup-dependencies.sh linux

# CLAP開発ツールのセットアップ
mkdir -p "$CLAP_TOOLS_BASE"
cd "$CLAP_TOOLS_BASE"
../../scripts/setup-clap-tools.sh linux "$CLAP_VALIDATOR_VERSION" "$CLAP_INFO_VERSION"
cd -

# vcpkg環境構築
./scripts/setup-vcpkg.sh linux

# aqtinstallインストール
python3 -m pip install --break-system-packages aqtinstall

# Qt6インストール
python3 -m aqt install-qt linux desktop 6.5.0 gcc_64 -O "$QT_BASE"

# CLAP Hostリポジトリの事前準備
if [ ! -d "$CLAP_HOST_BASE" ]; then
  git clone --recurse-submodules https://github.com/free-audio/clap-host.git "$CLAP_HOST_BASE"
  cd "$CLAP_HOST_BASE"
  git submodule update --init --recursive
  cd -
fi

# CLAP Hostのビルド
./scripts/build-clap-host.sh linux

echo "✅ devcontainer-oncreate.sh 完了"
