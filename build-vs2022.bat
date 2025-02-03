:::::::::::::::::::::::::::::::::::::::::::::::::::
::    Generate a Visual studio 2022 solution
:::::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
setlocal

set src_dir=%~dp0
set build_dir=%~dp0../build-vs2022
cmake -S %src_dir% -B %build_dir% --preset ci-vs2022
endlocal
