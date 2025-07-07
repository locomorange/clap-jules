#!/bin/bash
# GitHub Actions ワークフローを act で実行するスクリプト
# ローカル環境やCodespacesでCopilotワークフローと同じ環境を構築

set -e

# カラー出力
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Usage
show_usage() {
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --workflow WORKFLOW    Workflow file to run (default: copilot-setup-steps.yml)"
    echo "  --job JOB             Job name to run (default: copilot-setup-steps)"
    echo "  --dry-run             Show what would be executed without running"
    echo "  --list                List available workflows"
    echo "  --install-act         Install act if not present"
    echo "  -h, --help            Show this help"
}

# act インストール
install_act() {
    log_info "Installing act..."
    
    # OS検出
    OS=$(uname -s | tr '[:upper:]' '[:lower:]')
    ARCH=$(uname -m)
    
    case $ARCH in
        x86_64) ARCH="x86_64" ;;
        aarch64|arm64) ARCH="arm64" ;;
        *) log_error "Unsupported architecture: $ARCH"; return 1 ;;
    esac
    
    # GitHub CLI経由でインストールを試行
    if command -v gh &> /dev/null; then
        log_info "Installing act via GitHub CLI..."
        if gh extension install https://github.com/nektos/gh-act; then
            log_success "act installed via GitHub CLI"
            return 0
        fi
    fi
    
    # 直接ダウンロード
    log_info "Installing act directly..."
    ACT_VERSION=$(curl -s https://api.github.com/repos/nektos/act/releases/latest | grep '"tag_name"' | cut -d'"' -f4)
    
    if [ -z "$ACT_VERSION" ]; then
        log_error "Failed to get latest act version"
        return 1
    fi
    
    DOWNLOAD_URL="https://github.com/nektos/act/releases/download/${ACT_VERSION}/act_${OS}_${ARCH}.tar.gz"
    
    log_info "Downloading act from $DOWNLOAD_URL"
    curl -L "$DOWNLOAD_URL" | tar -xz
    
    # インストール
    if [ -f "act" ]; then
        sudo mv act /usr/local/bin/act
        chmod +x /usr/local/bin/act
        log_success "act installed to /usr/local/bin/act"
    else
        log_error "Failed to extract act binary"
        return 1
    fi
}

# ワークフロー一覧表示
list_workflows() {
    log_info "Available workflows:"
    find .github/workflows -name "*.yml" -o -name "*.yaml" | while read -r workflow; do
        echo "  $workflow"
        # ジョブ名を抽出
        if command -v yq &> /dev/null; then
            yq eval '.jobs | keys' "$workflow" 2>/dev/null | sed 's/^/    Job: /' || true
        fi
    done
}

# Docker確認
check_docker() {
    if ! command -v docker &> /dev/null; then
        log_error "Docker is required to run act"
        log_info "Please install Docker first"
        return 1
    fi
    
    if ! docker info &> /dev/null; then
        log_error "Docker daemon is not running"
        log_info "Please start Docker service"
        return 1
    fi
    
    log_success "Docker is available"
}

# ワークフロー実行
run_workflow() {
    local workflow_file="$1"
    local job_name="$2"
    local dry_run="$3"
    
    # ワークフローファイル確認
    if [ ! -f "$workflow_file" ]; then
        log_error "Workflow file not found: $workflow_file"
        return 1
    fi
    
    log_info "Running workflow: $workflow_file"
    log_info "Job: $job_name"
    
    # act コマンド構築
    local act_cmd="act"
    
    # ワークフロー指定
    act_cmd="$act_cmd -W $workflow_file"
    
    # ジョブ指定
    if [ -n "$job_name" ]; then
        act_cmd="$act_cmd --job $job_name"
    fi
    
    # 追加オプション
    act_cmd="$act_cmd --pull=false"  # イメージを毎回プルしない
    act_cmd="$act_cmd --rm"          # 実行後にコンテナ削除
    
    # Dry run
    if [ "$dry_run" = "true" ]; then
        act_cmd="$act_cmd --dryrun"
        log_info "Dry run mode - showing what would be executed:"
    fi
    
    log_info "Executing: $act_cmd"
    
    # 実行
    if eval "$act_cmd"; then
        log_success "Workflow completed successfully"
        return 0
    else
        local exit_code=$?
        log_error "Workflow failed with exit code: $exit_code"
        return $exit_code
    fi
}

# メイン処理
main() {
    local workflow_file=".github/workflows/copilot-setup-steps.yml"
    local job_name="copilot-setup-steps"
    local dry_run="false"
    local install_act_flag="false"
    
    # 引数解析
    while [[ $# -gt 0 ]]; do
        case $1 in
            --workflow)
                workflow_file="$2"
                shift 2
                ;;
            --job)
                job_name="$2"
                shift 2
                ;;
            --dry-run)
                dry_run="true"
                shift
                ;;
            --list)
                list_workflows
                exit 0
                ;;
            --install-act)
                install_act_flag="true"
                shift
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done
    
    log_info "CLAP GitHub Actions Workflow Runner"
    log_info "=================================="
    
    # act インストール確認
    if ! command -v act &> /dev/null || [ "$install_act_flag" = "true" ]; then
        install_act
    fi
    
    # Docker確認
    check_docker
    
    # ワークフロー実行
    run_workflow "$workflow_file" "$job_name" "$dry_run"
    
    log_success "Done!"
}

# エラーハンドリング
trap 'log_error "Script failed at line $LINENO"' ERR

# スクリプト直接実行時のみmainを呼び出し
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi