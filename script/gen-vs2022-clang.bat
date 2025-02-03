:::::::::::::::::::::::::::::::::::::::::::::::::::
::    Generate a Visual studio 2022 solution
:::::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
setlocal

SET script_path=%~dp0
SET src_dir=%script_path%..
set build_dir=%script_path%../../build-vs2022-clang
set install_dir=%script_path%../../install-vs2022-clang

cmake -S %src_dir% -B %build_dir% --preset vs2022-clang --install-prefix %install_dir%

endlocal
