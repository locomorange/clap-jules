# Screenshots and Verification Resources

このディレクトリには、CLAP開発プロセスで生成される検証・撮影リソースが保存されます。

## ファイル構成

### スクリーンショット
- `clap-host-screenshot.png` - CLAP Hostの動作スクリーンショット
- `clap-host-linux-screenshot.png` - Linux環境での動作確認
- `clap-host-macos-screenshot.png` - macOS環境での動作確認（必要に応じて）
- `clap-host-windows-screenshot.png` - Windows環境での動作確認（必要に応じて）

### ビルドログ
- `cmake-*-configure-log*.txt` - CMake設定ログ
- `cmake-*-build-log*.txt` - ビルド実行ログ
- `build-failed-*.txt` - ビルド失敗情報
- `found-executables-*.txt` - 発見された実行ファイル一覧

### 検証ログ
- `clap-validator-*.txt` - CLAP検証結果
- `clap-info-*.txt` - プラグイン情報抽出結果
- `clap-host-version*.txt` - CLAP Hostバージョン情報

### エラー診断
- `error-*.txt` - 各種エラー情報
- `diagnostic-*.txt` - 診断情報

## Coding Agentでの活用

### スクリーンショット確認
```bash
# 最新のスクリーンショット表示
ls -la screenshots/*.png

# 特定のスクリーンショット確認
file screenshots/clap-host-screenshot.png
```

### ログ確認
```bash
# ビルドエラーの確認
cat screenshots/cmake-*-build-log*.txt | tail -20

# CLAP検証結果確認
cat screenshots/clap-validator-*.txt
```

### エラー診断
```bash
# 失敗情報確認
find screenshots -name "*failed*" -type f | xargs cat

# 実行ファイル確認
cat screenshots/found-executables-*.txt
```

## 注意事項

- このディレクトリのファイルは自動生成されます
- 手動編集は避けてください
- CI/CDプロセスで利用されるため、削除時は注意が必要です
- 大容量ファイルの蓄積を防ぐため、定期的なクリーンアップを推奨します

## クリーンアップ

```bash
# 古いログファイルの削除（7日以上前）
find screenshots -name "*.txt" -mtime +7 -delete

# 古いスクリーンショットの削除（必要に応じて）
# find screenshots -name "*.png" -mtime +30 -delete
```