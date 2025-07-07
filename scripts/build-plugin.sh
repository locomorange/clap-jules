#!/bin/bash
set -e

echo "=== CLAP Plugin Build Script ==="

# プロジェクトルート取得
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# 環境変数読み込み
if [[ -f "$PROJECT_ROOT/.env" ]]; then
    echo "Loading environment from .env file..."
    source "$PROJECT_ROOT/.env"
else
    echo "Warning: .env file not found. Please run setup-environment.sh first."
fi

# サブモジュール初期化
echo "Initializing submodules..."
git submodule update --init --recursive

# ビルドディレクトリクリーンアップ（オプション）
if [ "$1" = "--clean" ]; then
    echo "Cleaning build directory..."
    rm -rf build/
fi

# CMake設定
echo "Configuring CMake..."

# 環境変数デバッグ情報
echo "Debug: Environment variables:"
echo "  PROJECT_ROOT: $PROJECT_ROOT"
echo "  VCPKG_ROOT: $VCPKG_ROOT"
echo "  CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
echo "  TRIPLET: $TRIPLET"

# CMakeオプションを構築
CMAKE_ARGS=()
CMAKE_ARGS+=("-B" "build")
CMAKE_ARGS+=("-DCMAKE_BUILD_TYPE=Release")

# ビルドシステムの選択
if command -v ninja &> /dev/null; then
    echo "Using Ninja build system"
    CMAKE_ARGS+=("-G" "Ninja")
elif command -v make &> /dev/null; then
    echo "Using Make build system"
    CMAKE_ARGS+=("-G" "Unix Makefiles")
else
    echo "Warning: Neither Ninja nor Make found, using default generator"
fi

# vcpkgツールチェーンファイルが存在する場合、使用する
if [[ -n "$CMAKE_TOOLCHAIN_FILE" && -f "$CMAKE_TOOLCHAIN_FILE" ]]; then
    echo "Using vcpkg toolchain: $CMAKE_TOOLCHAIN_FILE"
    CMAKE_ARGS+=("-DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE")
else
    echo "Warning: vcpkg toolchain file not found or not set"
    if [[ -n "$CMAKE_TOOLCHAIN_FILE" ]]; then
        echo "  CMAKE_TOOLCHAIN_FILE is set to: $CMAKE_TOOLCHAIN_FILE"
        echo "  File exists: $(ls -la "$CMAKE_TOOLCHAIN_FILE" 2>/dev/null || echo "No")"
    else
        echo "  CMAKE_TOOLCHAIN_FILE is not set"
    fi
fi

# ビルドツールの確認
echo "Debug: Build tools availability:"
echo "  cmake: $(command -v cmake || echo "Not found")"
echo "  make: $(command -v make || echo "Not found")"
echo "  ninja: $(command -v ninja || echo "Not found")"
echo "  gcc: $(command -v gcc || echo "Not found")"

echo "CMake command: cmake . ${CMAKE_ARGS[*]}"

# CMake実行
cmake . "${CMAKE_ARGS[@]}"

# ビルド実行
echo "Building plugin..."
cmake --build build --config Release

# プラグインファイル確認
echo "=== Build Results ==="
if [ -f "build/MyFirstClapPlugin.so" ]; then
    PLUGIN_FILE="build/MyFirstClapPlugin.so"
elif [ -f "build/Release/MyFirstClapPlugin.clap" ]; then
    PLUGIN_FILE="build/Release/MyFirstClapPlugin.clap"
elif [ -f "build/MyFirstClapPlugin.dylib" ]; then
    PLUGIN_FILE="build/MyFirstClapPlugin.dylib"
else
    echo "✗ Plugin file not found in expected locations!"
    echo "Build directory contents:"
    find build -name "*.so" -o -name "*.clap" -o -name "*.dylib" 2>/dev/null || echo "No plugin files found"
    exit 1
fi

echo "✓ Plugin built successfully: $PLUGIN_FILE"
ls -la "$PLUGIN_FILE"
file "$PLUGIN_FILE" 2>/dev/null || echo "File type check not available"

# CLAP検証（ツールが利用可能な場合）
echo "=== CLAP Validation ==="
if command -v clap-validator >/dev/null 2>&1; then
    echo "Running clap-validator..."
    clap-validator validate "$PLUGIN_FILE" || echo "Validation completed with warnings/errors"
    echo "✓ Plugin validation completed"
else
    echo "⚠ clap-validator not available - skipping validation"
fi

if command -v clap-info >/dev/null 2>&1; then
    echo "Running clap-info..."
    clap-info "$PLUGIN_FILE" --brief || echo "Info extraction completed"
    echo "✓ Plugin info extracted"
else
    echo "⚠ clap-info not available - skipping info extraction"
fi

# テスト実行
echo "=== Running Tests ==="
if ctest --test-dir build --output-on-failure; then
    echo "✓ All tests passed"
else
    echo "⚠ Some tests failed - check output above"
fi

echo "=== Build Summary ==="
echo "Plugin file: $PLUGIN_FILE"
echo "Build completed successfully!"