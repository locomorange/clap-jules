# Copilot Action移行計画

## 概要
既存のGitHub ActionsワークフローをCopilot Actionに移行し、CLAP hostのビルド・検証プロセスをCoding Agentが活用できるよう整備する。

## 現状分析

### 既存ワークフロー
- `build_and_test.yml`: メインビルド・テストパイプライン（825行）
- `copilot-setup-steps.yml`: 基本的なCopilot環境セットアップ（48行）

### 主要コンポーネント
1. **プラグインビルド**: CMakeベース、マルチプラットフォーム対応
2. **CLAP検証**: clap-validator (v0.3.2) + clap-info (v1.2.2)
3. **CLAP Hostテスト**: 統合テスト + スクリーンショット撮影
4. **依存関係管理**: vcpkg、Qt6、GUI libraries

## 移行戦略

### フェーズ1: 基本環境整備
既存の`copilot-setup-steps.yml`を拡張し、Coding Agentが必要とする全ツールを含める。

#### 追加要素:
- **ビルドツール**: CMake、Ninja、pkg-config
- **CLAP開発ツール**: clap-validator、clap-info
- **GUI依存関係**: X11 libraries、mesa-utils
- **スクリーンショット**: xvfb、imagemagick、scrot
- **CLAP Host依存**: vcpkg、RtMidi、RtAudio

### フェーズ2: ビルドプロセス標準化
Coding Agentが利用しやすい形でビルドスクリプトを整理。

#### 標準化項目:
- **統一ビルドコマンド**: プラットフォーム固有処理の抽象化
- **検証フロー**: clap-validator + clap-info実行
- **エラーハンドリング**: 失敗時の診断情報収集

### フェーズ3: CLAP Hostテスト環境
統合テスト用のCLAP Host環境を構築。

#### 構成要素:
- **CLAP Host**: 自動ビルド（vcpkg + Qt6）
- **テストプラグイン**: MyFirstClapPlugin
- **スクリーンショット**: Linux環境での自動撮影
- **比較機能**: 前回との差分検出

## 実装計画

### 1. 拡張copilot-setup-steps.yml

```yaml
name: "Enhanced Copilot Setup for CLAP Development"

on:
  workflow_dispatch:
  pull_request:
    paths:
      - .github/workflows/copilot-setup-steps.yml

env:
  CLAP_VALIDATOR_VERSION: '0.3.2'
  CLAP_INFO_VERSION: 'v1.2.2'

jobs:
  copilot-setup-steps:
    runs-on: ubuntu-latest
    timeout-minutes: 45
    
    permissions:
      contents: read

    steps:
      - name: Checkout code
        uses: actions/checkout@v4
        with:
          submodules: 'recursive'

      # ビルド環境セットアップ
      - name: Setup Build Environment
        run: |
          sudo apt-get update -y
          sudo apt-get install -y --no-install-recommends \
            build-essential cmake ninja-build pkg-config git curl \
            libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev \
            libgl1-mesa-dev libglu1-mesa-dev libxxf86vm-dev libxfixes-dev \
            mesa-utils xvfb x11-utils imagemagick scrot \
            zip unzip tar wget

      # CLAP開発ツール
      - name: Setup CLAP Tools
        run: |
          # clap-validator
          wget -O clap-validator.tar.gz https://github.com/free-audio/clap-validator/releases/download/${{ env.CLAP_VALIDATOR_VERSION }}/clap-validator-${{ env.CLAP_VALIDATOR_VERSION }}-ubuntu-18.04.tar.gz
          tar -xzf clap-validator.tar.gz
          chmod +x clap-validator
          
          # clap-info
          wget -O clap-info.zip https://github.com/free-audio/clap-info/releases/download/${{ env.CLAP_INFO_VERSION }}/clap-info-linux-x64.zip
          unzip -q clap-info.zip
          chmod +x clap-info
          
          echo "$PWD" >> $GITHUB_PATH

      # vcpkg環境構築
      - name: Setup vcpkg
        run: |
          git clone https://github.com/Microsoft/vcpkg.git vcpkg
          ./vcpkg/bootstrap-vcpkg.sh
          ./vcpkg/vcpkg install rtmidi rtaudio --triplet=x64-linux
          echo "VCPKG_ROOT=$(pwd)/vcpkg" >> $GITHUB_ENV

      # Qt6インストール
      - name: Install Qt
        uses: jurplel/install-qt-action@v4

      # CLAP Hostチェックアウト
      - name: Checkout CLAP Host
        run: |
          git clone --recurse-submodules https://github.com/free-audio/clap-host.git clap-host-repo
          echo "CLAP_HOST_PATH=$(pwd)/clap-host-repo" >> $GITHUB_ENV

      # 環境確認
      - name: Verify Environment
        run: |
          echo "=== Build Tools ==="
          cmake --version
          ninja --version
          gcc --version
          
          echo "=== CLAP Tools ==="
          ./clap-validator --help | head -5
          ./clap-info --help | head -5
          
          echo "=== vcpkg ==="
          echo "VCPKG_ROOT: $VCPKG_ROOT"
          
          echo "=== Qt ==="
          qmake --version
          
          echo "=== CLAP Host ==="
          ls -la clap-host-repo/
```

### 2. ビルドスクリプト統合

**scripts/build-plugin.sh**:
```bash
#!/bin/bash
set -e

echo "=== CLAP Plugin Build Script ==="

# サブモジュール初期化
git submodule update --init --recursive

# ビルド
cmake . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 検証
PLUGIN_FILE="build/MyFirstClapPlugin.so"
if [ -f "$PLUGIN_FILE" ]; then
    echo "✓ Plugin built successfully: $PLUGIN_FILE"
    
    # CLAP検証
    if command -v clap-validator >/dev/null 2>&1; then
        clap-validator validate "$PLUGIN_FILE"
        echo "✓ Plugin validation completed"
    fi
    
    if command -v clap-info >/dev/null 2>&1; then
        clap-info "$PLUGIN_FILE" --brief
        echo "✓ Plugin info extracted"
    fi
else
    echo "✗ Plugin build failed"
    exit 1
fi
```

**scripts/test-clap-host.sh**:
```bash
#!/bin/bash
set -e

echo "=== CLAP Host Integration Test ==="

PLUGIN_FILE="${1:-build/MyFirstClapPlugin.so}"
CLAP_HOST_PATH="${CLAP_HOST_PATH:-clap-host-repo}"

if [ ! -f "$PLUGIN_FILE" ]; then
    echo "✗ Plugin file not found: $PLUGIN_FILE"
    exit 1
fi

cd "$CLAP_HOST_PATH"

# CLAP Hostビルド
export CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake . -B builds/vcpkg-build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE" \
    -DVCPKG_TARGET_TRIPLET=x64-linux

cmake --build builds/vcpkg-build --config Release

# 実行ファイル確認
CLAP_HOST_EXEC=$(find builds -name "clap-host" -type f | head -1)
if [ -z "$CLAP_HOST_EXEC" ]; then
    echo "✗ CLAP Host executable not found"
    exit 1
fi

echo "✓ CLAP Host built: $CLAP_HOST_EXEC"

# テスト実行（仮想ディスプレイ）
export DISPLAY=:99
Xvfb :99 -screen 0 1024x768x24 &
XVFB_PID=$!
sleep 5

# スクリーンショット撮影
mkdir -p ../screenshots
timeout 30s bash -c "
    '$CLAP_HOST_EXEC' -p '$(pwd)/../$PLUGIN_FILE' &
    CLAP_PID=\$!
    sleep 15
    import -window root ../screenshots/clap-host-screenshot.png 2>/dev/null || scrot ../screenshots/clap-host-screenshot.png 2>/dev/null
    kill \$CLAP_PID 2>/dev/null || true
" || echo "Screenshot capture completed"

kill $XVFB_PID 2>/dev/null || true

if [ -f "../screenshots/clap-host-screenshot.png" ]; then
    echo "✓ Screenshot captured: screenshots/clap-host-screenshot.png"
else
    echo "⚠ Screenshot capture failed"
fi

echo "✓ CLAP Host integration test completed"
```

### 3. Coding Agent用ドキュメント

**CLAUDE.md**の拡張:
```markdown
# CLAP Development with Coding Agent

## Quick Commands

### Plugin Build & Test
```bash
# Build plugin
./scripts/build-plugin.sh

# Run unit tests  
ctest --test-dir build --output-on-failure

# CLAP Host integration test
./scripts/test-clap-host.sh
```

### Development Tools
- `clap-validator validate <plugin>` - Plugin validation
- `clap-info <plugin> --brief` - Plugin information
- CLAP Host path: `$CLAP_HOST_PATH`
- vcpkg root: `$VCPKG_ROOT`

### File Locations
- Plugin: `build/MyFirstClapPlugin.so`
- Screenshots: `screenshots/`
- Logs: `screenshots/*-log-*.txt`

## Common Tasks

### Build Plugin
1. Initialize submodules: `git submodule update --init --recursive`
2. Configure: `cmake . -B build -DCMAKE_BUILD_TYPE=Release`
3. Build: `cmake --build build --config Release`
4. Validate: `clap-validator validate build/MyFirstClapPlugin.so`

### Test with CLAP Host
1. Ensure plugin is built
2. Run: `./scripts/test-clap-host.sh`
3. Check screenshot: `screenshots/clap-host-screenshot.png`

### Troubleshooting
- Build logs: `screenshots/cmake-*-log-*.txt`
- Missing dependencies: Check vcpkg installation
- GUI issues: Ensure X11 libraries installed
```

## 期待される効果

### Coding Agentによる活用
1. **自動ビルド**: 標準化されたコマンドでプラグインビルド
2. **品質検証**: CLAP validatorによる自動検証
3. **統合テスト**: CLAP Hostでの動作確認
4. **視覚的確認**: スクリーンショットによる動作状況把握

### 開発効率向上
1. **環境構築**: 一発セットアップ
2. **一貫性**: プラットフォーム間の動作統一
3. **デバッグ**: 詳細ログとスクリーンショット
4. **CI/CD**: Copilot Actionとの自然な統合

## 移行スケジュール

### Week 1: 基盤整備
- [ ] 拡張copilot-setup-steps.yml実装
- [ ] ビルドスクリプト作成
- [ ] CLAUDE.md更新

### Week 2: テスト・調整
- [ ] 各種環境でのテスト実行
- [ ] エラーハンドリング改善
- [ ] ドキュメント改善

### Week 3: 本格運用
- [ ] 既存workflow無効化
- [ ] Copilot Action本格運用開始
- [ ] フィードバック収集・改善

## リスク対策

### 依存関係の複雑さ
- vcpkg使用による再現性確保
- Docker化も検討（必要に応じて）

### ビルド時間
- キャッシュ機能活用
- 段階的ビルド実装

### GUI環境
- 仮想ディスプレイによる安定化
- エラー時のフォールバック

## 成功指標

1. **Copilot Action実行成功率**: 95%以上
2. **ビルド時間**: 現在より20%短縮
3. **エラー解決時間**: 詳細ログによる50%短縮
4. **開発者満足度**: アンケートによる評価