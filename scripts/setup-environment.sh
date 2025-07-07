#!/bin/bash
# 共通環境セットアップスクリプト
# GitHub Actions、GitHub Codespaces、Copilot Coding Agent で共有
set -e

# 設定変数
export CLAP_VALIDATOR_VERSION=${CLAP_VALIDATOR_VERSION:-"0.3.2"}
export CLAP_INFO_VERSION=${CLAP_INFO_VERSION:-"v1.2.2"}
export SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

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

# プラットフォーム検出
detect_platform() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        PLATFORM="linux"
        ARCH=$(uname -m)
        if [[ "$ARCH" == "x86_64" ]]; then
            TRIPLET="x64-linux"
        else
            TRIPLET="arm64-linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        PLATFORM="macos"
        ARCH=$(uname -m)
        if [[ "$ARCH" == "arm64" ]]; then
            TRIPLET="arm64-osx"
        else
            TRIPLET="x64-osx"
        fi
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
        PLATFORM="windows"
        TRIPLET="x64-windows"
    else
        log_error "Unsupported platform: $OSTYPE"
        exit 1
    fi
    
    log_info "Detected platform: $PLATFORM ($ARCH)"
    export PLATFORM ARCH TRIPLET
}

# 基本ビルドツールのインストール
install_build_tools() {
    log_info "Installing build tools for $PLATFORM..."
    
    case $PLATFORM in
        "linux")
            sudo apt-get update -y
            sudo apt-get install -y --no-install-recommends \
                build-essential cmake ninja-build pkg-config git curl \
                zip unzip tar wget ca-certificates
            ;;
        "macos")
            # Homebrew経由でインストール
            if ! command -v brew &> /dev/null; then
                log_error "Homebrew not found. Please install Homebrew first."
                exit 1
            fi
            brew install cmake ninja pkg-config git curl
            ;;
        "windows")
            # Windows環境では通常MSBuildが使われるため、追加インストールは不要
            log_info "Windows build tools assumed to be pre-installed"
            ;;
    esac
    
    log_success "Build tools installed"
}

# GUI開発ライブラリのインストール (Linux専用)
install_gui_dependencies() {
    if [[ "$PLATFORM" == "linux" ]]; then
        log_info "Installing GUI dependencies for Linux..."
        
        # 複数回試行してインストール
        for attempt in 1 2 3; do
            if sudo apt-get install -y --no-install-recommends \
                libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev \
                libgl1-mesa-dev libglu1-mesa-dev libxxf86vm-dev libxfixes-dev \
                mesa-utils xvfb x11-utils imagemagick scrot; then
                log_success "GUI dependencies installed successfully"
                break
            else
                log_warning "Attempt $attempt failed, retrying..."
                sleep 2
            fi
        done
    else
        log_info "GUI dependencies not required for $PLATFORM"
    fi
}

# 環境検出
detect_environment() {
    # Codespaces環境の検出
    if [[ -n "$CODESPACES" || -n "$CODESPACE_NAME" ]]; then
        ENVIRONMENT="codespaces"
    # GitHub Actions環境の検出
    elif [[ -n "$GITHUB_ACTIONS" ]]; then
        ENVIRONMENT="github_actions"
    # その他のローカル環境
    else
        ENVIRONMENT="local"
    fi
    
    log_info "Detected environment: $ENVIRONMENT"
    export ENVIRONMENT
}

# Qt6のインストール
install_qt6() {
    log_info "Installing Qt6..."
    
    # Codespaces環境でのみaqtを使用
    if [[ "$ENVIRONMENT" == "codespaces" ]]; then
        case $PLATFORM in
            "linux"|"macos")
                # Python3とpipが必要
                if ! command -v python3 &> /dev/null; then
                    log_error "Python3 not found. Please install Python3 first."
                    return 1
                fi
                
                # aqtのインストール
                if ! command -v aqt &> /dev/null; then
                    log_info "Installing aqt (Another Qt installer) for Codespaces..."
                    python3 -m pip install --user aqtinstall
                    
                    # PATH更新
                    if [[ "$PLATFORM" == "linux" ]]; then
                        export PATH="$HOME/.local/bin:$PATH"
                    elif [[ "$PLATFORM" == "macos" ]]; then
                        export PATH="$HOME/Library/Python/3.*/bin:$PATH"
                    fi
                fi
                
                # Qt6インストール
                log_info "Installing Qt6 6.5.0 via aqt..."
                QT_DIR="$PROJECT_ROOT/Qt"
                mkdir -p "$QT_DIR"
                
                if [[ "$PLATFORM" == "linux" ]]; then
                    aqt install-qt linux desktop 6.5.0 gcc_64 -O "$QT_DIR"
                    export Qt6_DIR="$QT_DIR/6.5.0/gcc_64"
                    export PATH="$Qt6_DIR/bin:$PATH"
                elif [[ "$PLATFORM" == "macos" ]]; then
                    aqt install-qt mac desktop 6.5.0 clang_64 -O "$QT_DIR"
                    export Qt6_DIR="$QT_DIR/6.5.0/clang_64"
                    export PATH="$Qt6_DIR/bin:$PATH"
                fi
                
                log_success "Qt6 installed at $Qt6_DIR via aqt"
                ;;
            "windows")
                log_info "Qt6 installation on Windows Codespaces should be handled separately"
                ;;
        esac
    else
        # GitHub ActionsやローカルではSkip（別途セットアップされる想定）
        log_info "Qt6 installation skipped (expected to be handled by GitHub Actions or manually)"
        log_info "Environment: $ENVIRONMENT - Qt6 should be installed via jurplel/install-qt-action or manually"
    fi
}

# CLAPツールのインストール
install_clap_tools() {
    log_info "Installing CLAP tools..."
    
    cd "$PROJECT_ROOT"
    
    case $PLATFORM in
        "linux")
            # clap-validator
            if ! command -v clap-validator &> /dev/null; then
                log_info "Downloading clap-validator $CLAP_VALIDATOR_VERSION..."
                wget -O clap-validator.tar.gz "https://github.com/free-audio/clap-validator/releases/download/$CLAP_VALIDATOR_VERSION/clap-validator-$CLAP_VALIDATOR_VERSION-ubuntu-18.04.tar.gz"
                tar -xzf clap-validator.tar.gz
                chmod +x clap-validator
                rm clap-validator.tar.gz
            fi
            
            # clap-info
            if ! command -v clap-info &> /dev/null; then
                log_info "Downloading clap-info $CLAP_INFO_VERSION..."
                wget -O clap-info.zip "https://github.com/free-audio/clap-info/releases/download/$CLAP_INFO_VERSION/clap-info-linux-x64.zip"
                unzip -q clap-info.zip
                chmod +x clap-info
                rm clap-info.zip
            fi
            ;;
        "macos")
            # clap-validator
            if ! command -v clap-validator &> /dev/null; then
                log_info "Downloading clap-validator $CLAP_VALIDATOR_VERSION..."
                curl -L -o clap-validator.zip "https://github.com/free-audio/clap-validator/releases/download/$CLAP_VALIDATOR_VERSION/clap-validator-macos.zip"
                unzip -q clap-validator.zip
                chmod +x clap-validator
                rm clap-validator.zip
            fi
            
            # clap-info
            if ! command -v clap-info &> /dev/null; then
                log_info "Downloading clap-info $CLAP_INFO_VERSION..."
                curl -L -o clap-info.zip "https://github.com/free-audio/clap-info/releases/download/$CLAP_INFO_VERSION/clap-info-mac-universal.zip"
                unzip -q clap-info.zip
                chmod +x clap-info
                rm clap-info.zip
            fi
            ;;
        "windows")
            # clap-validator
            if ! command -v clap-validator.exe &> /dev/null; then
                log_info "Downloading clap-validator $CLAP_VALIDATOR_VERSION..."
                curl -L -o clap-validator.zip "https://github.com/free-audio/clap-validator/releases/download/$CLAP_VALIDATOR_VERSION/clap-validator-$CLAP_VALIDATOR_VERSION-windows.zip"
                unzip -q clap-validator.zip
                rm clap-validator.zip
            fi
            
            # clap-info
            if ! command -v clap-info.exe &> /dev/null; then
                log_info "Downloading clap-info $CLAP_INFO_VERSION..."
                curl -L -o clap-info.zip "https://github.com/free-audio/clap-info/releases/download/$CLAP_INFO_VERSION/clap-info-win-x64.zip"
                unzip -q clap-info.zip
                rm clap-info.zip
            fi
            ;;
    esac
    
    log_success "CLAP tools installed"
}

# vcpkgのセットアップ
setup_vcpkg() {
    log_info "Setting up vcpkg..."
    
    cd "$PROJECT_ROOT"
    
    # vcpkgクローンまたは更新
    if [ ! -d "vcpkg" ] || [ ! -f "vcpkg/.git/config" ]; then
        log_info "Cloning vcpkg repository..."
        rm -rf vcpkg 2>/dev/null || true
        git clone https://github.com/Microsoft/vcpkg.git vcpkg
    else
        log_info "vcpkg directory exists, updating..."
        cd vcpkg && git pull && cd ..
    fi
    
    # ブートストラップ
    log_info "Bootstrapping vcpkg..."
    if [[ "$PLATFORM" == "windows" ]]; then
        ./vcpkg/bootstrap-vcpkg.bat
    else
        ./vcpkg/bootstrap-vcpkg.sh
    fi
    
    # 依存関係インストール
    log_info "Installing CLAP Host dependencies..."
    ./vcpkg/vcpkg install rtmidi rtaudio --triplet="$TRIPLET"
    
    # 環境変数設定
    export VCPKG_ROOT="$PROJECT_ROOT/vcpkg"
    export CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
    
    log_success "vcpkg setup completed"
    log_info "VCPKG_ROOT: $VCPKG_ROOT"
    log_info "CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
}

# CLAP Hostリポジトリの準備
setup_clap_host() {
    log_info "Setting up CLAP Host repository..."
    
    cd "$PROJECT_ROOT"
    
    # CLAP Hostクローンまたは更新
    if [ ! -d "clap-host-repo" ] || [ ! -f "clap-host-repo/.git/config" ]; then
        log_info "Cloning CLAP Host repository..."
        rm -rf clap-host-repo 2>/dev/null || true
        git clone --recurse-submodules https://github.com/free-audio/clap-host.git clap-host-repo
    else
        log_info "CLAP Host directory exists, updating..."
        cd clap-host-repo
        git pull --recurse-submodules || echo "Git pull failed, continuing with existing version"
        cd ..
    fi
    
    export CLAP_HOST_PATH="$PROJECT_ROOT/clap-host-repo"
    
    log_success "CLAP Host repository prepared"
    log_info "CLAP_HOST_PATH: $CLAP_HOST_PATH"
}

# 環境検証
verify_environment() {
    log_info "Verifying environment setup..."
    
    # ビルドツール確認
    log_info "Build tools:"
    cmake --version | head -1
    
    if command -v ninja &> /dev/null; then
        ninja --version
    else
        log_warning "Ninja not found"
    fi
    
    if command -v gcc &> /dev/null; then
        gcc --version | head -1
    elif command -v clang &> /dev/null; then
        clang --version | head -1
    else
        log_warning "No C compiler found"
    fi
    
    # CLAPツール確認
    log_info "CLAP tools:"
    if command -v clap-validator &> /dev/null || command -v clap-validator.exe &> /dev/null; then
        clap-validator --help | head -1 || echo "clap-validator available"
    else
        log_warning "clap-validator not found"
    fi
    
    if command -v clap-info &> /dev/null || command -v clap-info.exe &> /dev/null; then
        clap-info --help | head -1 || echo "clap-info available"
    else
        log_warning "clap-info not found"
    fi
    
    # Qt6確認
    log_info "Qt6:"
    if command -v qmake &> /dev/null; then
        qmake --version | head -1
    elif [[ -n "$Qt6_DIR" && -f "$Qt6_DIR/bin/qmake" ]]; then
        "$Qt6_DIR/bin/qmake" --version | head -1
    else
        log_warning "Qt6/qmake not found"
    fi
    
    # vcpkg確認
    if [[ -n "$VCPKG_ROOT" && -d "$VCPKG_ROOT" ]]; then
        log_info "vcpkg: $VCPKG_ROOT"
        if [[ -d "$VCPKG_ROOT/installed/$TRIPLET" ]]; then
            log_info "vcpkg packages installed for $TRIPLET"
        else
            log_warning "vcpkg packages not found for $TRIPLET"
        fi
    else
        log_warning "vcpkg not configured"
    fi
    
    # CLAP Host確認
    if [[ -n "$CLAP_HOST_PATH" && -d "$CLAP_HOST_PATH" ]]; then
        log_info "CLAP Host: $CLAP_HOST_PATH"
    else
        log_warning "CLAP Host not configured"
    fi
    
    log_success "Environment verification completed"
}

# 環境変数をファイルに保存
save_environment() {
    log_info "Saving environment variables..."
    
    cat > "$PROJECT_ROOT/.env" << EOF
# CLAP開発環境設定
export PLATFORM="$PLATFORM"
export ARCH="$ARCH"
export TRIPLET="$TRIPLET"
export VCPKG_ROOT="$PROJECT_ROOT/vcpkg"
export CMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/vcpkg/scripts/buildsystems/vcpkg.cmake"
export CLAP_HOST_PATH="$PROJECT_ROOT/clap-host-repo"
export PATH="$PROJECT_ROOT:\$PATH"
EOF

    # Qt6環境変数を追加（存在する場合）
    if [[ -n "$Qt6_DIR" ]]; then
        cat >> "$PROJECT_ROOT/.env" << EOF
export Qt6_DIR="$Qt6_DIR"
export PATH="$Qt6_DIR/bin:\$PATH"
EOF
    fi
    
    log_success "Environment saved to .env file"
}

# メイン処理
main() {
    log_info "CLAP Development Environment Setup"
    log_info "=================================="
    
    # 引数処理
    SKIP_VCPKG=false
    SKIP_CLAP_HOST=false
    SKIP_CLAP_TOOLS=false
    SKIP_QT6=false
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --skip-vcpkg)
                SKIP_VCPKG=true
                shift
                ;;
            --skip-clap-host)
                SKIP_CLAP_HOST=true
                shift
                ;;
            --skip-clap-tools)
                SKIP_CLAP_TOOLS=true
                shift
                ;;
            --skip-qt6)
                SKIP_QT6=true
                shift
                ;;
            -h|--help)
                echo "Usage: $0 [options]"
                echo "Options:"
                echo "  --skip-vcpkg      Skip vcpkg setup"
                echo "  --skip-clap-host  Skip CLAP Host setup"
                echo "  --skip-clap-tools Skip CLAP tools installation"
                echo "  --skip-qt6        Skip Qt6 installation"
                echo "  -h, --help        Show this help"
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done
    
    # 環境セットアップ実行
    detect_platform
    detect_environment
    install_build_tools
    install_gui_dependencies
    
    if [[ "$SKIP_QT6" == false ]]; then
        install_qt6
    fi
    
    if [[ "$SKIP_CLAP_TOOLS" == false ]]; then
        install_clap_tools
    fi
    
    if [[ "$SKIP_VCPKG" == false ]]; then
        setup_vcpkg
    fi
    
    if [[ "$SKIP_CLAP_HOST" == false ]]; then
        setup_clap_host
    fi
    
    verify_environment
    save_environment
    
    log_success "Environment setup completed!"
    log_info "Source the environment: source .env"
}

# スクリプトが直接実行された場合のみmainを呼び出し
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi