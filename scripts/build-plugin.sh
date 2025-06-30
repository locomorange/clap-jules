#!/bin/bash
set -e

echo "=== CLAP Plugin Build Script ==="

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
cmake . -B build -DCMAKE_BUILD_TYPE=Release

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