@echo off
setlocal

SET script_path=%~dp0
SET src_path=%script_path%..

cmake -S %src_path% --preset vs2022 && ^
cmake --build --preset vs2022 --config Debug && ^
cmake --build --preset vs2022 --config Release && ^
cmake --build --preset vs2022 --config RelWithDebInfo

endlocal
