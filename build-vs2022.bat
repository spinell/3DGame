@echo off
setlocal
cmake -S %~dp0 -B %~dp0build/vs2022 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
endlocal
