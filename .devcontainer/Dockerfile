FROM fedora:44

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=1000

# ---------------------------------------------------------------------------
# Base development tools
# ---------------------------------------------------------------------------

RUN dnf -y update && \
    dnf -y install \
        gcc \
        gcc-c++ \
        gcc-g++ \
        gdb \
        git \
        make \
        ninja-build \
        cmake \
        meson \
        pkgconf-pkg-config \
        \
        boost-devel \
        \
        gtest-devel \
        gmock-devel \
        \
        freetype-devel \
        harfbuzz-devel \
        \
        libX11-devel \
        libXrandr-devel \
        libXcursor-devel \
        libXi-devel \
        libXinerama-devel \
        \
        mesa-libGL-devel \
        mesa-libEGL-devel \
        \
        libjpeg-turbo-devel \
        \
        flac-devel \
        libogg-devel \
        libvorbis-devel \
        \
        systemd-devel \
        libudev-devel \
        \
        openssl-devel \
        \
        pkgconf \
        \
        sudo \
        which \
        file \
        tar \
        gzip \
        unzip \
        \
        bash-completion \
    && dnf clean all \
    && rm -rf /var/cache/dnf

# ---------------------------------------------------------------------------
# Create the VS Code development user
# ---------------------------------------------------------------------------

RUN groupadd --gid ${USER_GID} ${USERNAME} && \
    useradd \
        --uid ${USER_UID} \
        --gid ${USER_GID} \
        --create-home \
        --shell /bin/bash \
        ${USERNAME} && \
    echo "${USERNAME} ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/${USERNAME} && \
    chmod 0440 /etc/sudoers.d/${USERNAME}

# ---------------------------------------------------------------------------
# Build and install SFML 3.0.2
#
# This is deliberately installed inside the container rather than using
# Fedora's SFML package. Fedora 44 provides SFML 2.6.x.
# ---------------------------------------------------------------------------

WORKDIR /tmp

RUN git clone \
        --branch 3.0.2 \
        --depth 1 \
        https://github.com/SFML/SFML.git \
        sfml

RUN cmake \
        -S sfml \
        -B sfml/build \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DBUILD_SHARED_LIBS=ON \
        -DSFML_BUILD_EXAMPLES=OFF \
        -DSFML_BUILD_TEST_SUITE=OFF

RUN cmake --build sfml/build

RUN cmake --install sfml/build

RUN rm -rf sfml

# Make sure libraries installed under /usr/local/lib64 are discoverable.
RUN echo "/usr/local/lib64" > /etc/ld.so.conf.d/local.conf && \
    echo "/usr/local/lib" >> /etc/ld.so.conf.d/local.conf && \
    ldconfig

# ---------------------------------------------------------------------------
# Make SFML 3 available through pkg-config as well.
#
# This is useful for Meson and makes:
#
#     pkg-config --modversion sfml-graphics
#
# resolve to SFML 3 inside the container.
# ---------------------------------------------------------------------------

ENV PKG_CONFIG_PATH="/usr/local/lib64/pkgconfig:/usr/local/lib/pkgconfig"

# ---------------------------------------------------------------------------
# Development environment
# ---------------------------------------------------------------------------

ENV PATH="/usr/local/bin:${PATH}"

WORKDIR /workspace

USER ${USERNAME}

CMD ["/bin/bash"]