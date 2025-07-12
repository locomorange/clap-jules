FROM ubuntu:latest

ARG USERNAME=ubuntu
ARG USER_UID=1000
ARG USER_GID=1000

ENV DEBIAN_FRONTEND=noninteractive

# 環境変数の設定（Copilot Setupと同じバージョン）
ENV CLAP_VALIDATOR_VERSION=0.3.2
ENV CLAP_INFO_VERSION=v1.2.2

# 必要なパッケージのインストールとユーザー設定
RUN apt-get update && apt-get install -y sudo curl wget \
    && usermod -aG sudo $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && rm -rf /var/lib/apt/lists/*

# スクリプトをコピー
COPY scripts/ /tmp/scripts/
RUN chmod +x /tmp/scripts/*.sh

# ビルド環境の包括的セットアップ（setup-dependencies.shと同等）
RUN /tmp/scripts/setup-dependencies.sh linux

# CLAP開発ツールのセットアップ
RUN mkdir -p /workspaces/clap-jules/tools/clap-tools && \
    cd /workspaces/clap-jules/tools/clap-tools && \
    /tmp/scripts/setup-clap-tools.sh linux "${CLAP_VALIDATOR_VERSION}" "${CLAP_INFO_VERSION}"

# デバッグ: CLAPツールディレクトリの内容を確認
RUN ls -la /workspaces/clap-jules/tools/clap-tools

# vcpkg環境構築とライブラリインストール
RUN /tmp/scripts/setup-vcpkg.sh linux /workspaces/clap-jules/tools/vcpkg

# makeとビルドツールの追加インストール
RUN apt-get update \
    && apt-get install -y make python3 python3-pip\
    && rm -rf /var/lib/apt/lists/*

# aqtinstallのインストール
RUN python3 -m pip install --break-system-packages aqtinstall \
    && echo "✅ aqtinstall installed successfully"

# Qt6のインストール（aqtinstallを使用）
RUN echo "🔧 Installing Qt6 using aqtinstall for devcontainer environment..." \
    && python3 -m aqt install-qt linux desktop 6.5.0 gcc_64 -O /workspaces/clap-jules/tools/qt \
    && echo "✅ Qt6 6.5.0 installed successfully via aqtinstall"

# Qt6環境変数の設定
ENV Qt6_DIR=/workspaces/clap-jules/tools/qt/6.5.0/gcc_64
ENV QT_PLUGIN_PATH=/workspaces/clap-jules/tools/qt/6.5.0/gcc_64/plugins
ENV QML2_IMPORT_PATH=/workspaces/clap-jules/tools/qt/6.5.0/gcc_64/qml
ENV PATH=/workspaces/clap-jules/tools/qt/6.5.0/gcc_64/bin:$PATH

# vcpkg環境変数の設定
ENV VCPKG_ROOT=/workspaces/clap-jules/tools/vcpkg
ENV PATH=/workspaces/clap-jules/tools/vcpkg:$PATH

# CLAP開発ツール環境変数の設定
ENV PATH=/workspaces/clap-jules/tools/clap-tools:$PATH

# CLAP Hostリポジトリの事前準備（スクリプトロジックをDockerfileに組み込み）
RUN git clone --recurse-submodules https://github.com/free-audio/clap-host.git /workspaces/clap-jules/tools/clap-host-template \
    && cd /workspaces/clap-jules/tools/clap-host-template \
    && git submodule update --init --recursive

# CLAP Hostのビルド実行
RUN /tmp/scripts/build-clap-host.sh linux

# CLAP Host関連の環境変数設定
ENV CLAP_HOST_TEMPLATE_PATH=/workspaces/clap-jules/tools/clap-host-template
ENV CLAP_HOST_PATH=/workspaces/clap-jules/clap-host-repo/builds
ENV CMAKE_TOOLCHAIN_FILE=libs/vcpkg/scripts/buildsystems/vcpkg.cmake

# ワークスペースの準備
WORKDIR /workspaces/clap-jules
RUN mkdir -p /workspaces/clap-jules \
    && chown -R $USERNAME:$USERNAME /workspaces/clap-jules

# 一時的なスクリプトファイルを削除
RUN rm -rf /tmp/scripts

USER $USERNAME
