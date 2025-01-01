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

export CMAKE_GENERATOR="Ninja"
export CC="gcc-14"
export CXX="g++-14"

cmake_options=()
cmake_options+=("-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake")
cmake_options+=("-DVCPKG_MANIFEST_INSTALL=ON")
cmake_options+=("-DVCPKG_TARGET_TRIPLET=x64-linux")

for build_type in Debug Release RelWithDebInfo
do
    export CMAKE_BUILD_TYPE="$build_type"

    src_dir="$script_folder"
    build_dir="$script_folder/build/$CMAKE_BUILD_TYPE-$CXX"
    install_dir="$script_folder/install/$CMAKE_BUILD_TYPE-$CXX"

    mkdir -p $build_dir
    cmake -S $script_folder -B $build_dir ${cmake_options[@]} && \
        cmake --build $build_dir && \
        cmake --install $build_dir --prefix $install_dir --verbose

done
