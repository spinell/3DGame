FROM ubuntu:24.04

ARG USERNAME=spinell
ARG USER_UID=1002
ARG USER_GID=$USER_UID

# curl zip unzip tar are required from vcpkg
RUN apt-get -y update && \
    apt-get -y install git && \
    apt-get -y --no-install-recommends install \
    cmake \
    ninja-build \
    binutils \
    gcc-14 \
    g++-14 \
    curl \
    zip \
    unzip \
    tar && \
    apt clean

# SDL2 will fail to build (vcpkg_fixup_pkgconfig)
# require to build SDL with alsa (autoconf libtool)
RUN apt-get -y --no-install-recommends install \
    pkg-config \
    autoconf \
    libtool && \
    apt clean

#######################################################################
# SDL2 dependencies
# https://github.com/libsdl-org/SDL/blob/main/docs/README-linux.md
#######################################################################

# libasound2-dev      (libasound.so.2)       : alsa driver
# libpulse-dev        (libpulse.so.0)        : pulseaudio driver
# libpipewire-0.3-dev (libpipewire-0.3.so.0) : pipewire driver
# libaudio-dev        (libaudio.so.2)        : nas driver
# libjack-dev         ( libjack.so.0)        : jack driver
# libsndio-dev        (libsndio.so.7)        : sndio driver
#
# This is required for SDL to build with X11 support
# - libx11-dev libxext-dev libxft-dev
# Does libxft-dev required ?
RUN apt-get -y --no-install-recommends install \
    libasound2-dev \
    libpulse-dev \
    libpipewire-0.3-dev \
    libaudio-dev \
    libjack-dev \
    libsndio-dev \
    libx11-dev \
    libxext-dev \
    libxft-dev \
    libwayland-dev \
    libxkbcommon-dev \
    libegl1-mesa-dev && \
    apt clean

# Create the user
RUN groupadd --gid $USER_GID $USERNAME && \
    useradd --uid $USER_UID \
            --gid $USER_GID \
            --create-home $USERNAME \
            --shell /bin/bash

USER $USERNAME
ENV VCPKG_ROOT="/home/$USERNAME/vcpkg"
ENV PATH="$VCPKG_ROOT:$PATH"

# Install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git --branch 2024.12.16 $VCPKG_ROOT && \
    $VCPKG_ROOT/bootstrap-vcpkg.sh -disableMetrics

ENTRYPOINT ["/bin/bash"]
