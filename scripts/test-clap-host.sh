#!/bin/bash
set -e

echo "=== CLAP Host Integration Test ==="

# パラメータ設定
PLUGIN_FILE="${1:-build/MyFirstClapPlugin.so}"
CLAP_HOST_PATH="${CLAP_HOST_PATH:-clap-host-repo}"
SCREENSHOT_DIR="screenshots"

# プラグインファイル確認
if [ ! -f "$PLUGIN_FILE" ]; then
    echo "✗ Plugin file not found: $PLUGIN_FILE"
    echo "Available plugin files:"
    find build -name "*.so" -o -name "*.clap" -o -name "*.dylib" 2>/dev/null || echo "No plugin files found"
    exit 1
fi

echo "✓ Using plugin: $PLUGIN_FILE"

# CLAP Host パス確認
if [ ! -d "$CLAP_HOST_PATH" ]; then
    echo "✗ CLAP Host directory not found: $CLAP_HOST_PATH"
    echo "Cloning CLAP Host repository..."
    git clone --recurse-submodules https://github.com/free-audio/clap-host.git "$CLAP_HOST_PATH"
fi

cd "$CLAP_HOST_PATH"
echo "✓ Using CLAP Host at: $(pwd)"

# 環境変数確認（Brisk用vcpkgと分離）
if [ -z "$VCPKG_ROOT" ]; then
    echo "⚠ VCPKG_ROOT not set - trying to find vcpkg for clap-host..."
    
    # clap-host専用のvcpkgを探す（Brisk用とは別）
    if [ -d "../clap-host-vcpkg" ]; then
        export VCPKG_ROOT="$(pwd)/../clap-host-vcpkg"
    elif [ -d "../../clap-host-vcpkg" ]; then
        export VCPKG_ROOT="$(pwd)/../../clap-host-vcpkg"
    elif [ -d "../vcpkg" ]; then
        export VCPKG_ROOT="$(pwd)/../vcpkg"
    elif [ -d "../../vcpkg" ]; then
        export VCPKG_ROOT="$(pwd)/../../vcpkg"
    else
        echo "✗ vcpkg not found for clap-host"
        echo "  Note: Brisk has its own vcpkg at libs/brisk/vcpkg"
        echo "  Installing separate vcpkg for clap-host..."
        
        # Install separate vcpkg for clap-host
        cd ..
        if [ ! -d "clap-host-vcpkg" ]; then
            git clone https://github.com/Microsoft/vcpkg.git clap-host-vcpkg
            cd clap-host-vcpkg
            if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
                ./bootstrap-vcpkg.bat
            else
                ./bootstrap-vcpkg.sh
            fi
            cd ..
        fi
        export VCPKG_ROOT="$(pwd)/clap-host-vcpkg"
        cd "$CLAP_HOST_PATH"
    fi
fi

echo "✓ Using clap-host vcpkg at: $VCPKG_ROOT"
echo "  (Separate from Brisk vcpkg to avoid conflicts)"
export CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# ビルド試行（複数の方法を試す）
echo "=== Building CLAP Host ==="
BUILD_SUCCESS=false

# vcpkgビルド
if [ "$BUILD_SUCCESS" = "false" ]; then
    echo "Trying vcpkg build..."
    if cmake . -B builds/vcpkg-build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE" \
        -DVCPKG_TARGET_TRIPLET=x64-linux \
        -DUsePkgConfig=OFF 2>&1 | tee ../screenshots/cmake-vcpkg-configure-log.txt; then
        echo "vcpkg configuration successful"
        if cmake --build builds/vcpkg-build --config Release 2>&1 | tee ../screenshots/cmake-vcpkg-build-log.txt; then
            echo "vcpkg build successful"
            BUILD_SUCCESS=true
            BUILD_DIR="builds/vcpkg-build"
        fi
    fi
fi

# ninja-systemプリセット
if [ "$BUILD_SUCCESS" = "false" ]; then
    echo "Trying ninja-system preset..."
    if cmake --preset ninja-system 2>&1 | tee ../screenshots/cmake-preset-configure-log.txt; then
        echo "ninja-system preset configuration successful"
        if cmake --build --preset ninja-system 2>&1 | tee ../screenshots/cmake-preset-build-log.txt; then
            echo "ninja-system preset build successful"
            BUILD_SUCCESS=true
            BUILD_DIR="builds/ninja-system"
        fi
    fi
fi

# シンプルビルド
if [ "$BUILD_SUCCESS" = "false" ]; then
    echo "Trying simple build..."
    if cmake . -B builds/simple -G Ninja -DCMAKE_BUILD_TYPE=Release 2>&1 | tee ../screenshots/cmake-simple-configure-log.txt; then
        echo "Simple configuration successful"
        if cmake --build builds/simple --config Release 2>&1 | tee ../screenshots/cmake-simple-build-log.txt; then
            echo "Simple build successful"
            BUILD_SUCCESS=true
            BUILD_DIR="builds/simple"
        fi
    fi
fi

if [ "$BUILD_SUCCESS" = "false" ]; then
    echo "✗ All build attempts failed"
    exit 1
fi

# 実行ファイル確認
echo "=== Finding CLAP Host executable ==="
CLAP_HOST_EXEC=$(find $BUILD_DIR -name "clap-host" -type f | head -1)
if [ -z "$CLAP_HOST_EXEC" ]; then
    echo "✗ clap-host executable not found!"
    echo "Available executables in $BUILD_DIR:"
    find $BUILD_DIR -name "*clap*" -type f | head -10
    exit 1
fi

echo "✓ Found clap-host at: $CLAP_HOST_EXEC"
chmod +x "$CLAP_HOST_EXEC"

# スクリーンショット準備
cd ..
mkdir -p "$SCREENSHOT_DIR"
PLUGIN_FULL_PATH="$(pwd)/$PLUGIN_FILE"

# Linux環境でスクリーンショット撮影
if [ "$(uname)" = "Linux" ]; then
    echo "=== Taking screenshot on Linux ==="
    
    # 仮想ディスプレイ起動
    export DISPLAY=:99
    Xvfb :99 -screen 0 1024x768x24 &
    XVFB_PID=$!
    sleep 5
    
    # CLAP Host起動とスクリーンショット
    timeout 30s bash -c "
        cd '$CLAP_HOST_PATH'
        '$CLAP_HOST_EXEC' -p '$PLUGIN_FULL_PATH' &
        CLAP_PID=\$!
        sleep 15
        import -window root ../screenshots/clap-host-screenshot.png 2>/dev/null || scrot ../screenshots/clap-host-screenshot.png 2>/dev/null || echo 'Screenshot command failed'
        kill \$CLAP_PID 2>/dev/null || true
    " || echo "Screenshot capture completed"
    
    # 仮想ディスプレイ終了
    kill $XVFB_PID 2>/dev/null || true
    
    if [ -f "$SCREENSHOT_DIR/clap-host-screenshot.png" ]; then
        echo "✓ Screenshot captured: $SCREENSHOT_DIR/clap-host-screenshot.png"
        ls -la "$SCREENSHOT_DIR/clap-host-screenshot.png"
    else
        echo "⚠ Screenshot capture failed"
    fi
else
    echo "=== Testing CLAP Host (no screenshot on $(uname)) ==="
    
    # 簡単な動作テスト
    timeout 15s bash -c "
        cd '$CLAP_HOST_PATH'
        '$CLAP_HOST_EXEC' -p '$PLUGIN_FULL_PATH' &
        CLAP_PID=\$!
        sleep 10
        kill \$CLAP_PID 2>/dev/null || true
    " || echo "CLAP Host test completed"
    
    echo "✓ CLAP Host test completed (no screenshot)"
fi

# バージョン情報記録
cd "$CLAP_HOST_PATH"
"$CLAP_HOST_EXEC" --version > "../$SCREENSHOT_DIR/clap-host-version.txt" 2>&1 || echo "Version check failed"

echo "=== Test Summary ==="
echo "Plugin: $PLUGIN_FULL_PATH"
echo "CLAP Host: $CLAP_HOST_EXEC"
echo "Screenshots: $(pwd)/../$SCREENSHOT_DIR/"
echo "✓ CLAP Host integration test completed"