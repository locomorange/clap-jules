#!/bin/bash
set -e

echo "=== GitHub Actions Workflow Execution with act ==="

# ヘルプ表示
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -j, --job <job-name>     Run specific job (default: copilot-setup-steps)"
    echo "  -l, --list               List available workflows and jobs"
    echo "  -h, --help               Show this help"
    echo "  --dry-run                Show what would be executed without running"
    echo "  --verbose                Enable verbose output"
    echo ""
    echo "Examples:"
    echo "  $0                       # Run copilot-setup-steps"
    echo "  $0 -j copilot-setup-steps"
    echo "  $0 -l                    # List all jobs"
    echo "  $0 --dry-run             # Show execution plan"
}

# デフォルト設定
JOB_NAME="copilot-setup-steps"
DRY_RUN=false
VERBOSE=false
LIST_ONLY=false

# コマンドライン引数の解析
while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--job)
            JOB_NAME="$2"
            shift 2
            ;;
        -l|--list)
            LIST_ONLY=true
            shift
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# 利用可能なワークフローとジョブの一覧表示
if [ "$LIST_ONLY" = true ]; then
    echo "Available workflows and jobs:"
    act -l
    exit 0
fi

# Docker環境チェック
check_docker() {
    if ! command -v docker &> /dev/null; then
        echo "Error: Docker is not installed or not in PATH"
        exit 1
    fi
    
    if ! docker info >/dev/null 2>&1; then
        echo "Error: Docker daemon is not running or not accessible"
        echo "Please ensure Docker is running and you have appropriate permissions"
        exit 1
    fi
}

# act実行環境チェック
check_act() {
    if ! command -v act &> /dev/null; then
        echo "Error: act is not installed or not in PATH"
        echo "Please install act: https://github.com/nektos/act"
        exit 1
    fi
}

# 実行前チェック
echo "Checking environment..."
check_docker
check_act

# .github/workflows ディレクトリの存在確認
if [ ! -d ".github/workflows" ]; then
    echo "Error: .github/workflows directory not found"
    echo "Please ensure you're in the root of a repository with GitHub Actions workflows"
    exit 1
fi

# Dry runモード
if [ "$DRY_RUN" = true ]; then
    echo "=== DRY RUN MODE ==="
    echo "Would execute: act -j $JOB_NAME"
    echo ""
    echo "Available workflows:"
    ls -la .github/workflows/
    echo ""
    echo "To actually run, remove --dry-run flag"
    exit 0
fi

# Verboseオプションの設定
VERBOSE_FLAG=""
if [ "$VERBOSE" = true ]; then
    VERBOSE_FLAG="--verbose"
fi

# act実行
echo "Executing GitHub Actions job: $JOB_NAME"
echo "Working directory: $(pwd)"
echo ""

# actコマンドの実行
act_command="act -j $JOB_NAME $VERBOSE_FLAG --container-architecture linux/amd64"

echo "Running: $act_command"
echo "================================"

if eval "$act_command"; then
    echo ""
    echo "=== SUCCESS ==="
    echo "GitHub Actions job '$JOB_NAME' completed successfully!"
else
    echo ""
    echo "=== FAILED ==="
    echo "GitHub Actions job '$JOB_NAME' failed"
    echo ""
    echo "Troubleshooting tips:"
    echo "1. Check if the workflow file exists and is valid"
    echo "2. Ensure Docker has enough resources (memory, disk space)"
    echo "3. Try running with --verbose flag for more details"
    echo "4. Check act documentation: https://github.com/nektos/act"
    exit 1
fi
