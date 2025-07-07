# GitHub Actions ワークフロー

このディレクトリには、CLAP開発用のGitHub Actionsワークフローが含まれています。

## ワークフロー一覧

### 1. `copilot-setup-steps.yml`
- **用途**: GitHub Copilot Coding Agent用の環境セットアップ
- **機能**: 
  - 完全なCLAP開発環境構築
  - プラグインビルド・テスト
  - CLAP Host統合テスト
  - スクリーンショット撮影

### 2. `build_and_test.yml`
- **用途**: CI/CDパイプライン
- **機能**:
  - マルチプラットフォームビルド
  - 単体テスト実行
  - CLAP検証
  - CLAP Host統合テスト

## ローカル実行

これらのワークフローは `act` を使用してローカルでも実行できます：

```bash
# act のインストール
./scripts/run-copilot-workflow.sh --install-act

# Copilotワークフローの実行
./scripts/run-copilot-workflow.sh

# ビルド・テストワークフローの実行
./scripts/run-copilot-workflow.sh --workflow .github/workflows/build_and_test.yml --job build-plugin

# ドライラン（実行内容確認）
./scripts/run-copilot-workflow.sh --dry-run

# 利用可能なワークフロー一覧
./scripts/run-copilot-workflow.sh --list
```

## act 設定

### `.actrc`
プロジェクトルートの `.actrc` でact全体の設定を行います。

### `.github/workflows/.actrc`
このディレクトリの `.actrc` でワークフロー固有の設定を行います。

## 環境統一

- **GitHub Actions**: クラウドで自動実行
- **Codespaces**: `.devcontainer/run-workflow.sh` でワークフロー実行
- **ローカル**: `./scripts/run-copilot-workflow.sh` でワークフロー実行

すべての環境で同じワークフローを実行することで、完全に統一された開発環境を提供します。