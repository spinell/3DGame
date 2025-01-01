@echo off
setlocal
cmake -S . -B build/vs2022 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
endlocal
