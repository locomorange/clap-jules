# CLAP Development with Coding Agent

## 概要
CLAP (CLever Audio Plugin) 開発環境とCoding Agentが連携するための設定・手順書です。

## 環境設定

### 開発ツール
- `clap-validator` (v0.3.2) - プラグイン検証ツール
- `clap-info` (v1.2.2) - プラグイン情報取得ツール
- CMake + Ninja - ビルドシステム
- vcpkg - C++パッケージマネージャー
- Qt6 - CLAP Host用UI フレームワーク

### 環境変数
```bash
export VCPKG_ROOT=/path/to/vcpkg
export CLAP_HOST_PATH=tools/clap-host
export CMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
```

## 主要コマンド

### プラグインビルド
```bash
# 基本ビルド手順
git submodule update --init --recursive
cmake . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# ビルド成果物確認
ls -la build/MyFirstClapPlugin.so  # Linux
ls -la build/Release/MyFirstClapPlugin.clap  # Windows
```

### テスト実行
```bash
# 単体テスト
ctest --test-dir build --output-on-failure

# CLAP検証
clap-validator validate build/MyFirstClapPlugin.so
clap-info build/MyFirstClapPlugin.so --brief
```

### CLAP Host統合テスト
```bash
# CLAP Host環境構築
cd tools/clap-host
cmake . -B builds/vcpkg-build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE \
    -DVCPKG_TARGET_TRIPLET=x64-linux

cmake --build builds/vcpkg-build --config Release

# プラグインテスト
CLAP_HOST_EXEC=$(find builds -name "clap-host" -type f | head -1)
$CLAP_HOST_EXEC -p build/MyFirstClapPlugin.so
```

### スクリーンショット撮影 (Linux)
```bash
# 仮想ディスプレイでスクリーンショット撮影
export DISPLAY=:99
Xvfb :99 -screen 0 1024x768x24 &
XVFB_PID=$!

# CLAP Host起動してスクリーンショット
timeout 30s bash -c "
    '$CLAP_HOST_EXEC' -p 'build/MyFirstClapPlugin.so' &
    CLAP_PID=\$!
    sleep 15
    import -window root screenshots/clap-host-screenshot.png
    kill \$CLAP_PID 2>/dev/null || true
"

kill $XVFB_PID
```

## ファイル構成

### 重要なファイル
- `CMakeLists.txt` - メインビルド設定
- `my_plugin.cpp` - プラグイン実装
- `my_plugin.h` - プラグインヘッダー
- `test/test_my_plugin.cpp` - 単体テスト

### ビルド成果物
- `build/MyFirstClapPlugin.so` - Linux用プラグイン
- `build/Release/MyFirstClapPlugin.clap` - Windows用プラグイン
- `build/MyFirstClapPlugin.dylib` - macOS用プラグイン

### ログ・スクリーンショット
- `screenshots/` - スクリーンショット保存先
- `screenshots/*-log-*.txt` - ビルドログ

## 依存関係

### 必須ライブラリ
```bash
# Linux
sudo apt-get install -y \
    build-essential cmake ninja-build pkg-config \
    libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev libxxf86vm-dev libxfixes-dev \
    mesa-utils xvfb x11-utils imagemagick scrot
```

### vcpkg パッケージ
```bash
./vcpkg/vcpkg install rtmidi rtaudio --triplet=x64-linux
```

## トラブルシューティング

### ビルドエラー
1. **サブモジュール未初期化**
   ```bash
   git submodule update --init --recursive
   ```

2. **CMake設定エラー**
   ```bash
   rm -rf build/
   cmake . -B build -DCMAKE_BUILD_TYPE=Release
   ```

3. **依存関係不足**
   ```bash
   # vcpkg再インストール
   ./vcpkg/vcpkg install rtmidi rtaudio --triplet=x64-linux --force
   ```

### CLAP Host問題
1. **実行ファイル見つからない**
   ```bash
   find tools/clap-host -name "clap-host*" -type f
   ```

2. **GUI起動エラー**
   ```bash
   # 仮想ディスプレイ確認
   ps aux | grep Xvfb
   export DISPLAY=:99
   ```

3. **プラグイン読み込みエラー**
   ```bash
   # プラグインファイル確認
   file build/MyFirstClapPlugin.so
   ldd build/MyFirstClapPlugin.so  # 依存関係確認
   ```

## よくある作業パターン

### 1. 新機能開発時
```bash
# 1. プラグイン修正
vim my_plugin.cpp

# 2. ビルド・テスト
cmake --build build --config Release
ctest --test-dir build --output-on-failure

# 3. CLAP検証
clap-validator validate build/MyFirstClapPlugin.so
clap-info build/MyFirstClapPlugin.so --brief
```

### 2. バグ修正時
```bash
# 1. テスト追加
vim test/test_my_plugin.cpp

# 2. 修正実装
vim my_plugin.cpp

# 3. 検証
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

### 3. リリース前確認
```bash
# 1. フルビルド
rm -rf build/
cmake . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 2. 全テスト実行
ctest --test-dir build --output-on-failure

# 3. CLAP検証
clap-validator validate build/MyFirstClapPlugin.so

# 4. CLAP Host統合テスト
# (上記のCLAP Host統合テスト手順を実行)

# 5. スクリーンショット確認
ls -la screenshots/clap-host-screenshot.png
```

## Coding Agent利用時の注意点

### 推奨作業フロー
1. 現在のブランチ・状態確認
2. サブモジュール更新
3. ビルド実行
4. テスト実行
5. CLAP検証
6. 必要に応じてCLAP Host統合テスト

### 避けるべき操作
- サブモジュールの直接変更
- build/ディレクトリの直接編集
- 環境変数の永続的変更

### 効率的な利用方法
- ビルドエラー時はログファイルを確認
- CLAP Host問題時はスクリーンショットを確認
- 依存関係問題時はvcpkgログを確認