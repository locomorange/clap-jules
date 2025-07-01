# CLAP Development Verification Guide

## 概要
CLAP開発における検証プロセス全体を管理するためのガイドです。Coding Agentが効率的に品質確認を行うための手順とリソースを整理しています。

## 検証レベル

### レベル1: 基本ビルド検証
最低限の動作確認を行うレベル

```bash
# クイック検証
./scripts/build-plugin.sh
```

**確認項目:**
- [ ] サブモジュール初期化完了
- [ ] CMake設定成功
- [ ] プラグインビルド成功
- [ ] 単体テスト実行成功

### レベル2: CLAP準拠検証
CLAP仕様準拠を確認するレベル

```bash
# CLAP検証
clap-validator validate build/MyFirstClapPlugin.so
clap-info build/MyFirstClapPlugin.so --brief
```

**確認項目:**
- [ ] CLAP仕様準拠チェック通過
- [ ] プラグイン情報正常取得
- [ ] エラー・警告の許容範囲確認

### レベル3: 統合テスト検証
CLAP Hostでの実際の動作を確認するレベル

```bash
# 統合テスト
./scripts/test-clap-host.sh
```

**確認項目:**
- [ ] CLAP Hostビルド成功
- [ ] プラグイン読み込み成功
- [ ] GUI表示確認（スクリーンショット）
- [ ] 基本動作確認

## プラットフォーム別検証

### Linux (推奨プラットフォーム)
```bash
# フル検証シーケンス
./scripts/build-plugin.sh --clean
./scripts/test-clap-host.sh

# スクリーンショット確認
ls -la screenshots/clap-host-screenshot.png
```

### Windows
```bash
# Windows専用検証 (WSL/Git Bash)
cmake . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
# プラグイン: build/Release/MyFirstClapPlugin.clap
```

### macOS
```bash
# macOS検証
./scripts/build-plugin.sh
# プラグイン: build/MyFirstClapPlugin.dylib
# Note: スクリーンショットは手動確認推奨
```

## エラー対応フローチャート

### ビルドエラー
```
ビルド失敗
    ↓
サブモジュール確認
    ↓
git submodule update --init --recursive
    ↓
依存関係確認
    ↓
sudo apt-get install [必要なパッケージ]
    ↓
再ビルド
```

### CLAP検証エラー
```
CLAP検証失敗
    ↓
プラグインファイル確認
    ↓
file build/MyFirstClapPlugin.so
    ↓
プラグイン実装確認
    ↓
my_plugin.cpp修正
    ↓
再ビルド・再検証
```

### CLAP Host統合エラー
```
CLAP Host起動失敗
    ↓
依存関係確認
    ↓
vcpkg install rtmidi rtaudio
    ↓
CLAP Host再ビルド
    ↓
環境変数確認
    ↓
export DISPLAY=:99 (Linux)
    ↓
再実行
```

## 自動化された検証ポイント

### GitHub Actions連携
- **PR作成時**: 基本ビルド検証 (レベル1)
- **マージ前**: CLAP準拠検証 (レベル2)
- **リリース前**: 統合テスト検証 (レベル3)

### Copilot Action活用
```yaml
# .github/workflows/enhanced-copilot-setup-steps.yml
# - 環境自動構築
# - ツール自動インストール
# - 検証自動実行
```

## 検証結果の解釈

### 成功パターン
```
✓ Plugin built successfully: build/MyFirstClapPlugin.so
✓ Plugin validation completed
✓ Plugin info extracted
✓ All tests passed
✓ CLAP Host integration test completed
✓ Screenshot captured: screenshots/clap-host-screenshot.png
```

### 警告パターン
```
⚠ clap-validator not available - skipping validation
⚠ Some tests failed - check output above
⚠ Screenshot capture failed
⚠ CLAP Host test completed (no screenshot)
```

### エラーパターン
```
✗ Plugin file not found!
✗ Plugin build failed
✗ clap-host executable not found!
✗ All build attempts failed
```

## 品質基準

### 必須基準
- [ ] ビルドエラーなし
- [ ] 単体テスト全通過
- [ ] CLAP validator警告レベル以下
- [ ] メモリリークなし

### 推奨基準
- [ ] CLAP validator完全通過
- [ ] CLAP Host正常起動確認
- [ ] スクリーンショット取得成功
- [ ] マルチプラットフォーム動作確認

### 理想基準
- [ ] パフォーマンステスト通過
- [ ] 複数DAWでの動作確認
- [ ] 長時間動作安定性確認
- [ ] ドキュメント完備

## Coding Agent向けTips

### 効率的な検証手順
1. **段階的検証**: レベル1→2→3の順番で実行
2. **ログ確認**: `screenshots/`ディレクトリの内容を必ず確認
3. **エラー時**: 関連ログファイルを組み合わせて原因特定
4. **スクリーンショット**: 視覚的な動作確認を重視

### よく使うコマンド
```bash
# 環境確認
cmake --version && clap-validator --help | head -1

# クイック検証
./scripts/build-plugin.sh && echo "Build OK"

# フル検証
./scripts/build-plugin.sh --clean && ./scripts/test-clap-host.sh

# ログ確認
find screenshots -name "*.txt" -mtime -1 | head -5

# スクリーンショット確認
ls -la screenshots/*.png | tail -3
```

### トラブルシューティング
```bash
# 一般的な問題解決
git submodule update --init --recursive  # サブモジュール
rm -rf build/ && ./scripts/build-plugin.sh --clean  # ビルドクリア
export VCPKG_ROOT=/path/to/vcpkg  # 環境変数
sudo apt-get update && sudo apt-get install -y [パッケージ]  # 依存関係
```

## 継続的改善

### 検証プロセスの改善
- 検証時間の短縮
- エラー診断の高度化
- 自動化レベルの向上

### ツールチェーンのアップデート
- CLAP仕様の最新化
- 検証ツールのバージョンアップ
- 新しい検証項目の追加

このガイドを活用して、効率的で確実なCLAP開発を進めてください。