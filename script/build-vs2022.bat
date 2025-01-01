@echo off
setlocal

SET script_path=%~dp0
SET src_path=%script_path%..

cmake -S %src_path% --preset ci-vs2022 && ^
cmake --build --preset ci-vs2022 --config Debug && ^
cmake --build --preset ci-vs2022 --config Release && ^
cmake --build --preset ci-vs2022 --config RelWithDebInfo

cmake -S %src_path% --preset ci-vs2022-clang && ^
cmake --build --preset ci-vs2022-clang --config Debug && ^
cmake --build --preset ci-vs2022-clang --config Release && ^
cmake --build --preset ci-vs2022-clang --config RelWithDebInfo

endlocal
