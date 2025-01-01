#!/bin/bash

#apt-get install libltdl-dev

# sdl warning
#  - You will need to install Xorg dependencies to use feature x11:
#    sudo apt install libx11-dev libxft-dev libxext-dev
#  - You will need to install Wayland dependencies to use feature wayland:
#    sudo apt install libwayland-dev libxkbcommon-dev libegl1-mesa-dev
#  - You will need to install ibus dependencies to use feature ibus:
#    sudo apt install libibus-1.0-dev

echo "The script you are running has:"
echo "basename: [$(basename "$0")]"
echo "dirname : [$(dirname "$0")]"
echo "pwd     : [$(pwd)]"

# this path of this script
script_folder=$(dirname "$0")

cmake_options=()
cmake_options+=("-DVCPKG_MANIFEST_INSTALL=ON")
cmake_options+=("-DVCPKG_TARGET_TRIPLET=x64-linux")

cmake --preset ci-ninja-multi-gcc14 && \
  cmake --build --preset ci-ninja-multi-gcc14 --config Debug && \
  cmake --build --preset ci-ninja-multi-gcc14 --config Release && \
  cmake --build --preset ci-ninja-multi-gcc14 --config RelWithDebInfo
