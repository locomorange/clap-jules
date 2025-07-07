# CLAP Development Environment Setup

このプロジェクトは、GitHub Actions、GitHub Codespaces、GitHub Copilot Coding Agentで統一された開発環境を提供します。

## 共通化されたセットアップスクリプト

### 1. 基本環境セットアップ (`scripts/setup-environment.sh`)
- プラットフォーム検出（Linux、macOS、Windows）
- 環境検出（Codespaces、GitHub Actions、ローカル）
- ビルドツール（CMake、Ninja、GCC/Clang）
- GUI開発依存関係（Linux用）
- Qt6インストール（Codespaces のみ aqt 使用）
- CLAPツール（clap-validator、clap-info）
- vcpkg パッケージマネージャー
- CLAP Host リポジトリ

### 2. 統合開発環境セットアップ (`scripts/setup-clap-dev.sh`)
- 完全な開発環境構築
- プロジェクト初期化
- 依存関係検証
- 初回ビルドテスト
- VSCode設定生成

## 各環境での使用方法

### GitHub Codespaces / DevContainer

devcontainerが起動時にactを使用してCopilotワークフローを直接実行します：

```json
{
  "postCreateCommand": "bash .devcontainer/run-workflow.sh"
}
```

この方法により、`copilot-setup-steps.yml` を**そのまま実行**し、完全に同一の環境を提供します。

手動でワークフローを実行：
```bash
# Copilotワークフローを直接実行
./scripts/run-copilot-workflow.sh

# 特定のワークフローを実行
./scripts/run-copilot-workflow.sh --workflow .github/workflows/build_and_test.yml

# ドライラン（実行内容の確認）
./scripts/run-copilot-workflow.sh --dry-run

# ワークフロー一覧表示
./scripts/run-copilot-workflow.sh --list
```

### GitHub Actions

ワークフローで共有スクリプト + jurplel/install-qt-action（キャッシュ対応）：

```yaml
- name: Setup Development Environment
  run: |
    bash scripts/setup-environment.sh --skip-qt6

- name: Install Qt6
  uses: jurplel/install-qt-action@v4
  with:
    version: '6.5.0'
    cache: true
```

### GitHub Copilot Coding Agent

Copilot用の専用ワークフローで完全な環境構築とテストを実行：

```yaml
- name: Setup CLAP Development Environment
  run: |
    bash scripts/setup-clap-dev.sh --quick

- name: Install Qt6
  uses: jurplel/install-qt-action@v4
  with:
    version: '6.5.0'
    cache: true

- name: Test Plugin Build
  run: |
    ./scripts/build-plugin.sh

- name: Test CLAP Host Integration  
  run: |
    ./scripts/test-clap-host.sh
```

## 環境変数

セットアップ後、以下の環境変数が設定されます：

```bash
# 環境変数を読み込み
source .env

# 主要な環境変数
PLATFORM=linux          # linux, macos, windows
ARCH=x86_64             # x86_64, arm64
TRIPLET=x64-linux       # vcpkg triplet
VCPKG_ROOT=./vcpkg      # vcpkg installation path
CLAP_HOST_PATH=./clap-host-repo  # CLAP Host path
Qt6_DIR=./Qt/6.5.0/gcc_64       # Qt6 installation path (when available)
CMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
```

## 利用可能なコマンド

### 開発ワークフロー

```bash
# 1. GitHub Actions ワークフローで環境セットアップ（推奨）
./scripts/run-copilot-workflow.sh

# 2. 環境変数読み込み
source .env

# 3. プラグインビルド
./scripts/build-plugin.sh

# 4. CLAP Host統合テスト
./scripts/test-clap-host.sh

# 従来のスクリプトも使用可能
./scripts/setup-clap-dev.sh
./scripts/setup-environment.sh
```

### スクリプトオプション

```bash
# setup-environment.sh オプション
--skip-vcpkg      # vcpkg setup をスキップ
--skip-clap-host  # CLAP Host setup をスキップ
--skip-clap-tools # CLAP tools installation をスキップ
--skip-qt6        # Qt6 installation をスキップ

# setup-clap-dev.sh オプション
--skip-build      # 初回ビルドテストをスキップ
--skip-verify     # 依存関係検証をスキップ
--quick           # クイックセットアップ（一部スキップ）
```

## ファイル構造

```
.
├── scripts/
│   ├── setup-environment.sh    # 基本環境セットアップ
│   ├── setup-clap-dev.sh      # 統合開発環境セットアップ
│   ├── build-plugin.sh        # プラグインビルド
│   └── test-clap-host.sh       # CLAP Host テスト
├── .devcontainer/
│   └── devcontainer.json       # Codespaces設定
├── .github/workflows/
│   ├── build_and_test.yml      # ビルド・テストワークフロー
│   └── copilot-setup-steps.yml # Copilot専用ワークフロー
├── .env                        # 環境変数（生成される）
├── vcpkg/                      # vcpkg（生成される）
├── clap-host-repo/             # CLAP Host（生成される）
├── Qt/                         # Qt6（生成される）
└── build/                      # ビルド成果物
```

## トラブルシューティング

### 環境変数が読み込まれない

```bash
# 環境変数を手動で読み込み
source .env

# または環境変数を再生成
./scripts/setup-environment.sh
```

### ビルドが失敗する

```bash
# ビルドディレクトリをクリーンアップ
rm -rf build/

# 環境を再セットアップ
./scripts/setup-clap-dev.sh

# 依存関係を再インストール
./scripts/setup-environment.sh
```

### vcpkgの問題

```bash
# vcpkgを再インストール
rm -rf vcpkg/
./scripts/setup-environment.sh
```

## 環境の特徴

### 共通化の利点

1. **一貫性**: 全環境で同じセットアップスクリプトを使用
2. **保守性**: 1つのスクリプトを更新すれば全環境に反映
3. **効率性**: 重複コードを削減
4. **信頼性**: 環境間の差異を最小化

### プラットフォーム対応

- **Linux**: フル機能対応（GUI、スクリーンショット含む）
- **macOS**: ビルドとテスト対応
- **Windows**: 基本ビルド対応

### Qt6 インストール戦略

- **Codespaces**: aqt を使用（ワークフローステップを再現）
- **GitHub Actions/Copilot**: jurplel/install-qt-action を使用（アクションキャッシュ）
- **ローカル**: 手動インストールまたはスキップ

### キャッシュ対応

- **DevContainer**: vcpkg、CLAP Host、Qt6のボリュームマウント
- **GitHub Actions**: jurplel/install-qt-action の cache: true でQt6キャッシュ、ビルド成果物とdependencyのキャッシュ

この統一環境により、どの環境でも同じ開発体験を提供できます。