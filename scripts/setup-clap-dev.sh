#!/bin/bash
# CLAP開発環境統合セットアップスクリプト
# プロジェクトの完全な開発環境を構築
set -e

# 設定
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# カラー出力
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# 基本環境セットアップ
setup_base_environment() {
    log_info "Setting up base environment..."
    
    # 基本環境セットアップスクリプト実行
    if [[ -f "$SCRIPT_DIR/setup-environment.sh" ]]; then
        source "$SCRIPT_DIR/setup-environment.sh"
        log_success "Base environment setup completed"
    else
        log_error "setup-environment.sh not found"
        exit 1
    fi
}

# プロジェクト固有の初期化
initialize_project() {
    log_info "Initializing project..."
    
    cd "$PROJECT_ROOT"
    
    # サブモジュール初期化
    log_info "Initializing git submodules..."
    git submodule update --init --recursive
    
    # ディレクトリ作成
    log_info "Creating project directories..."
    mkdir -p build
    mkdir -p screenshots
    
    # スクリプト実行権限設定
    log_info "Setting up script permissions..."
    chmod +x scripts/*.sh 2>/dev/null || true
    
    log_success "Project initialized"
}

# 依存関係検証
verify_dependencies() {
    log_info "Verifying dependencies..."
    
    local missing_deps=()
    
    # 必須ツール確認
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi
    
    if ! command -v git &> /dev/null; then
        missing_deps+=("git")
    fi
    
    # プラットフォーム固有の確認
    if [[ "$PLATFORM" == "linux" ]]; then
        if ! command -v ninja &> /dev/null; then
            missing_deps+=("ninja")
        fi
        
        if ! dpkg -l | grep -q libx11-dev; then
            missing_deps+=("libx11-dev")
        fi
    elif [[ "$PLATFORM" == "macos" ]]; then
        if ! command -v brew &> /dev/null; then
            missing_deps+=("homebrew")
        fi
    fi
    
    # 不足している依存関係の報告
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        log_warning "Missing dependencies: ${missing_deps[*]}"
        log_info "Please install missing dependencies and run this script again"
        return 1
    fi
    
    log_success "All dependencies verified"
}

# 初回ビルドテスト
test_build() {
    log_info "Testing initial build..."
    
    cd "$PROJECT_ROOT"
    
    # CMake設定
    log_info "Configuring CMake..."
    if cmake . -B build -DCMAKE_BUILD_TYPE=Release; then
        log_success "CMake configuration successful"
    else
        log_error "CMake configuration failed"
        return 1
    fi
    
    # ビルド実行
    log_info "Building project..."
    if cmake --build build --config Release; then
        log_success "Build successful"
    else
        log_error "Build failed"
        return 1
    fi
    
    # プラグインファイル確認
    local plugin_files=(
        "build/MyFirstClapPlugin.so"
        "build/Release/MyFirstClapPlugin.clap"
        "build/MyFirstClapPlugin.dylib"
    )
    
    local plugin_found=false
    for plugin_file in "${plugin_files[@]}"; do
        if [[ -f "$plugin_file" ]]; then
            log_success "Plugin built successfully: $plugin_file"
            plugin_found=true
            break
        fi
    done
    
    if [[ "$plugin_found" == false ]]; then
        log_error "No plugin file found after build"
        return 1
    fi
    
    # テスト実行
    log_info "Running tests..."
    if ctest --test-dir build --output-on-failure; then
        log_success "All tests passed"
    else
        log_warning "Some tests failed"
    fi
}

# 開発ツールの動作確認
verify_dev_tools() {
    log_info "Verifying development tools..."
    
    # CLAPツールの動作確認
    if command -v clap-validator &> /dev/null || command -v clap-validator.exe &> /dev/null; then
        log_success "clap-validator is available"
    else
        log_warning "clap-validator not found"
    fi
    
    if command -v clap-info &> /dev/null || command -v clap-info.exe &> /dev/null; then
        log_success "clap-info is available"
    else
        log_warning "clap-info not found"
    fi
    
    # vcpkg確認
    if [[ -n "$VCPKG_ROOT" && -d "$VCPKG_ROOT" ]]; then
        log_success "vcpkg is configured: $VCPKG_ROOT"
    else
        log_warning "vcpkg not configured"
    fi
    
    # CLAP Host確認
    if [[ -n "$CLAP_HOST_PATH" && -d "$CLAP_HOST_PATH" ]]; then
        log_success "CLAP Host is available: $CLAP_HOST_PATH"
    else
        log_warning "CLAP Host not configured"
    fi
}

# 設定ファイル生成
generate_config_files() {
    log_info "Generating configuration files..."
    
    cd "$PROJECT_ROOT"
    
    # VSCode設定
    if [[ ! -d ".vscode" ]]; then
        mkdir -p .vscode
        
        # settings.json
        cat > .vscode/settings.json << EOF
{
    "cmake.configureArgs": [
        "-DCMAKE_BUILD_TYPE=Release"
    ],
    "cmake.buildDirectory": "\${workspaceFolder}/build",
    "files.exclude": {
        "build/": true,
        "vcpkg/": true,
        "clap-host-repo/": true
    }
}
EOF
        
        # launch.json
        cat > .vscode/launch.json << EOF
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug CLAP Plugin",
            "type": "cppdbg",
            "request": "launch",
            "program": "\${workspaceFolder}/clap-host-repo/builds/vcpkg-build/clap-host",
            "args": ["-p", "\${workspaceFolder}/build/MyFirstClapPlugin.so"],
            "stopAtEntry": false,
            "cwd": "\${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
EOF
        
        log_success "VSCode configuration generated"
    fi
    
    # Git設定
    if [[ ! -f ".gitignore" ]]; then
        cat > .gitignore << EOF
# Build directories
build/
builds/
vcpkg_installed/

# vcpkg
vcpkg/
.vcpkg-root

# CLAP Host
clap-host-repo/

# Screenshots and logs
screenshots/
*.log

# Environment
.env

# IDE
.vscode/
*.swp
*.swo
*~

# OS
.DS_Store
Thumbs.db
EOF
        log_success "Git ignore file generated"
    fi
}

# 使用方法の表示
show_usage() {
    log_info "Development Environment Setup Summary"
    log_info "===================================="
    
    echo
    echo "Environment is ready for CLAP development!"
    echo
    echo "Available commands:"
    echo "  ./scripts/build-plugin.sh       - Build and validate plugin"
    echo "  ./scripts/test-clap-host.sh     - Test with CLAP Host"
    echo "  ./scripts/setup-environment.sh  - Re-setup environment"
    echo
    echo "Environment variables:"
    echo "  VCPKG_ROOT: $VCPKG_ROOT"
    echo "  CLAP_HOST_PATH: $CLAP_HOST_PATH"
    echo "  CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
    echo
    echo "To activate the environment in new shells:"
    echo "  source .env"
    echo
    echo "Development workflow:"
    echo "  1. Edit source files (my_plugin.cpp, my_plugin.h)"
    echo "  2. Run ./scripts/build-plugin.sh"
    echo "  3. Test with ./scripts/test-clap-host.sh"
    echo "  4. Commit your changes"
    echo
}

# メイン処理
main() {
    log_info "CLAP Development Environment Setup"
    log_info "=================================="
    
    # 引数処理
    SKIP_BUILD=false
    SKIP_VERIFY=false
    QUICK_MODE=false
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --skip-build)
                SKIP_BUILD=true
                shift
                ;;
            --skip-verify)
                SKIP_VERIFY=true
                shift
                ;;
            --quick)
                QUICK_MODE=true
                shift
                ;;
            -h|--help)
                echo "Usage: $0 [options]"
                echo "Options:"
                echo "  --skip-build   Skip initial build test"
                echo "  --skip-verify  Skip dependency verification"
                echo "  --quick        Quick setup (skip build and some verifications)"
                echo "  -h, --help     Show this help"
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done
    
    # クイックモードの場合は一部をスキップ
    if [[ "$QUICK_MODE" == true ]]; then
        SKIP_BUILD=true
        SKIP_VERIFY=true
    fi
    
    # セットアップ実行
    setup_base_environment
    initialize_project
    
    if [[ "$SKIP_VERIFY" == false ]]; then
        verify_dependencies
    fi
    
    generate_config_files
    
    if [[ "$SKIP_BUILD" == false ]]; then
        test_build
    fi
    
    verify_dev_tools
    show_usage
    
    log_success "Complete development environment setup finished!"
}

# スクリプトが直接実行された場合のみmainを呼び出し
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi