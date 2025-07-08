FROM mcr.microsoft.com/devcontainers/base:ubuntu-22.04

# 基本パッケージとDockerのインストール
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl \
    wget \
    git \
    ca-certificates \
    gnupg \
    lsb-release \
    && rm -rf /var/lib/apt/lists/*

# Dockerのインストール（actで必要）
RUN curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | tee /etc/apt/sources.list.d/docker.list > /dev/null \
    && apt-get update \
    && apt-get install -y docker-ce-cli \
    && rm -rf /var/lib/apt/lists/*

# actのインストール
RUN curl -s https://raw.githubusercontent.com/nektos/act/master/install.sh | bash

# 作業ディレクトリの設定
WORKDIR /workspaces/clap-jules

# GitHub Actionsワークフローの実行用スクリプト
RUN echo '#!/bin/bash\n\
set -e\n\
echo "=== Running GitHub Actions workflow with act ==="\n\
\n\
# Docker daemon が利用可能かチェック\n\
if ! docker info >/dev/null 2>&1; then\n\
    echo "Error: Docker daemon is not available. Please ensure Docker is running."\n\
    echo "In GitHub Codespaces, Docker should be available by default."\n\
    exit 1\n\
fi\n\
\n\
# GitHub Actionsワークフローを実行\n\
echo "Executing copilot-setup-steps workflow..."\n\
act -j copilot-setup-steps --container-architecture linux/amd64 --pull=false || {\n\
    echo "Act execution failed. Trying with medium image..."\n\
    act -j copilot-setup-steps --container-architecture linux/amd64 --pull=false -P ubuntu-latest=nektos/act-environments-ubuntu:18.04\n\
}\n\
\n\
echo "=== GitHub Actions workflow completed ==="\n\
' > /usr/local/bin/run-copilot-setup.sh \
    && chmod +x /usr/local/bin/run-copilot-setup.sh

# 環境変数の設定
ENV DOCKER_BUILDKIT=1

# 便利なエイリアスとスクリプト
RUN echo '#!/bin/bash\n\
echo "Available commands:"\n\
echo "  run-copilot-setup.sh  - Run the copilot setup workflow with act"\n\
echo "  act -l                - List available workflows"\n\
echo "  act -j <job-name>     - Run specific job"\n\
echo ""\n\
echo "Current workflows:"\n\
if [ -d ".github/workflows" ]; then\n\
    ls -la .github/workflows/\n\
else\n\
    echo "  No .github/workflows directory found"\n\
fi\n\
' > /usr/local/bin/show-help.sh \
    && chmod +x /usr/local/bin/show-help.sh

# PostCreateCommandで実行される初期化スクリプト
RUN echo '#!/bin/bash\n\
set -e\n\
echo "=== DevContainer Post-Create Setup ==="\n\
\n\
# Git submodulesの更新\n\
if [ -f ".gitmodules" ]; then\n\
    echo "Updating git submodules..."\n\
    git submodule update --init --recursive\n\
fi\n\
\n\
# スクリプトに実行権限を付与\n\
if [ -d "scripts" ]; then\n\
    echo "Setting execute permissions on scripts..."\n\
    find scripts -name "*.sh" -exec chmod +x {} \;\n\
fi\n\
\n\
echo "=== Setup completed ==="\n\
echo "Run '\''show-help.sh'\'' to see available commands"\n\
' > /usr/local/bin/post-create-setup.sh \
    && chmod +x /usr/local/bin/post-create-setup.sh

# デフォルトユーザーをvscodeに設定
USER vscode

# シェルの設定
RUN echo 'alias ll="ls -la"' >> ~/.bashrc \
    && echo 'alias act-list="act -l"' >> ~/.bashrc \
    && echo 'alias act-copilot="act -j copilot-setup-steps"' >> ~/.bashrc
