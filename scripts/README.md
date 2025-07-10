# CI Scripts Documentation

このディレクトリには、GitHub Actionsワークフローをモジュール化した共通スクリプトが含まれています。

## スクリプト一覧

### `setup-dependencies.sh`
**用途**: プラットフォーム別の依存関係セットアップ  
**使用法**: `./setup-dependencies.sh <linux|windows|macos>`  
**機能**: 
- ビルドツールとGUIライブラリのインストール
- プラットフォーム固有の依存関係解決

### `setup-clap-tools.sh`
**用途**: CLAP検証ツールのダウンロードとセットアップ  
**使用法**: `./setup-clap-tools.sh <linux|windows|macos> [validator_version] [info_version]`  
**機能**:
- clap-validatorとclap-infoのダウンロード
- プラットフォーム別バイナリの配置
- 実行権限の設定

### `verify-plugin-build.sh`
**用途**: プラグインビルド成果物の検証  
**使用法**: `./verify-plugin-build.sh <linux|windows|macos> [build_dir]`  
**機能**:
- プラットフォーム別プラグインファイルの存在確認
- ファイル情報の表示
- GitHub Actions出力変数の設定

### `verify-plugin-build-optional.sh`
**用途**: オプションのプラグインビルド検証（失敗許容）  
**使用法**: `./verify-plugin-build-optional.sh <linux|windows|macos> [build_dir]`  
**機能**:
- 環境セットアップ用のオプション検証
- ビルド失敗でも正常終了
- Copilot Setup用に最適化

### `validate-clap-plugin.sh`
**用途**: CLAPプラグインの検証  
**使用法**: `./validate-clap-plugin.sh <linux|windows|macos> [artifacts_dir]`  
**機能**:
- clap-validatorによる検証実行
- clap-infoによる情報抽出
- プラットフォーム別ファイルパス解決

### `setup-vcpkg.sh`
**用途**: vcpkgパッケージマネージャーのセットアップ  
**使用法**: `./setup-vcpkg.sh <linux|windows|macos> [vcpkg_dir]`  
**機能**:
- vcpkgのクローンとブートストラップ
- RtMidiとRtAudioのインストール
- CMAKE_TOOLCHAIN_FILEの設定

### `verify-plugin-artifact.sh`
**用途**: ダウンロードしたプラグインアーティファクトの検証  
**使用法**: `./verify-plugin-artifact.sh <linux|windows|macos> [artifacts_dir]`  
**機能**:
- アーティファクトファイルの存在確認
- プラグインファイルパスの環境変数設定
- フォールバック検索の実行

### `build-clap-host.sh`
**用途**: CLAP Hostのビルド  
**使用法**: `./build-clap-host.sh <linux|windows|macos> [clap_host_dir] [vcpkg_root]`  
**機能**:
- プラットフォーム別ビルド戦略の実行
- vcpkgツールチェーンの使用
- 複数のビルド方法のフォールバック

### `test-clap-host.sh`
**用途**: CLAP Hostを使用したプラグインテスト  
**使用法**: `./test-clap-host.sh <linux|windows|macos> [clap_host_dir] [plugin_file] [timeout]`  
**機能**:
- プラグインのロードテスト
- Linuxでのスクリーンショット撮影
- 仮想ディスプレイの管理

### `upload-screenshot.sh`
**用途**: スクリーンショットのGitHubリリースへのアップロードとPRコメント  
**使用法**: `./upload-screenshot.sh <github_token> <repository> <pr_number> <run_id> <sha> <screenshot_hash>`  
**機能**:
- GitHubリリースへのファイルアップロード
- Pull Requestへのコメント投稿
- スクリーンショット比較情報の提供

### `prepare-clap-host.sh`
**用途**: CLAP Hostリポジトリの準備  
**使用法**: `./prepare-clap-host.sh [clap_host_dir]`  
**機能**:
- CLAP Hostリポジトリのクローンまたは更新
- 環境変数の設定
- リポジトリの状態確認

### `verify-environment.sh`
**用途**: 開発環境の包括的検証  
**使用法**: `./verify-environment.sh`  
**機能**:
- ビルドツールの動作確認
- CLAPツールの検証
- vcpkgとQt6の状態確認
- GUI環境とスクリーンショットツールの確認

### `environment-summary.sh`
**用途**: 環境セットアップの完了サマリー表示  
**使用法**: `./environment-summary.sh`  
**機能**:
- 設定済み環境の要約表示
- 利用可能なコマンドの一覧
- Copilot開発に必要な情報の提示

## 使用例

### ローカル開発での使用
```bash
# 依存関係のセットアップ
./scripts/ci/setup-dependencies.sh linux

# CLAPツールのセットアップ
./scripts/ci/setup-clap-tools.sh linux

# vcpkgのセットアップ
./scripts/ci/setup-vcpkg.sh linux

# CLAP Hostの準備
./scripts/ci/prepare-clap-host.sh

# プラグインビルドの検証
./scripts/ci/verify-plugin-build.sh linux

# 環境の検証
./scripts/ci/verify-environment.sh

# CLAPツールでの検証
./scripts/ci/validate-clap-plugin.sh linux
```

### Copilot Setupでの使用
Copilot Setup Stepsワークフローでは、以下のスクリプトを順次実行：

1. `setup-dependencies.sh` - 基本依存関係のインストール
2. `setup-clap-tools.sh` - CLAP検証ツールのセットアップ
3. `setup-vcpkg.sh` - vcpkgとライブラリのインストール
4. **キャッシュチェック** - CLAP Hostビルド成果物のキャッシュ確認
5. `prepare-clap-host.sh` - CLAP Hostリポジトリの準備（キャッシュミス時のみ）
6. `build-clap-host.sh` - CLAP Hostのビルド（キャッシュミス時のみ、オプション）
7. `verify-environment.sh` - 環境の包括的検証
8. **オプション**: プラグインビルドテスト（失敗許容）
9. `environment-summary.sh` - セットアップ完了サマリー

**キャッシュ機能**:
- CLAP Hostのビルド成果物をキャッシュして高速化
- キャッシュキー: `clap-host-copilot-setup-{OS}-{ハッシュ}-{バージョン}`
- キャッシュヒット時はリポジトリクローンとビルドをスキップ

**注意**: Copilot Setupではプラグインビルドは必須ではありません。環境セットアップが主目的で、ビルドは検証のためのオプションテストです。

### CI/CDでの統合
新しいワークフローファイル `build_and_test_refactored.yml` では、これらのスクリプトを活用して：

1. **コードの重複を削減**: 共通処理をスクリプト化
2. **保守性の向上**: ロジックの集約による変更の簡素化
3. **テスタビリティの向上**: スクリプトの個別テストが可能
4. **可読性の向上**: ワークフローファイルの簡素化

## 利点

- **再利用性**: 他のワークフローでも同じスクリプトを使用可能
- **テスタビリティ**: 各スクリプトを個別にテスト可能
- **保守性**: ロジックの変更が一箇所で済む
- **可読性**: ワークフローファイルがより簡潔に
- **デバッグ性**: 問題の特定と修正が容易

## 移行手順

1. 現在の `build_and_test.yml` を `build_and_test_legacy.yml` にリネーム
2. `build_and_test_refactored.yml` を `build_and_test.yml` にリネーム
3. 動作確認後、レガシーファイルを削除
