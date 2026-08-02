@echo off
setlocal
set "FLECS_SCRIPT_DIR=%~dp0"
set "FLECS_PROJECT_ROOT=%FLECS_SCRIPT_DIR%..\.."
pushd "%FLECS_PROJECT_ROOT%"
python "%FLECS_SCRIPT_DIR%ConvertTests.py" --project-root "%FLECS_PROJECT_ROOT%" %*
set "FLECS_EXIT=%ERRORLEVEL%"
popd
endlocal & exit /b %FLECS_EXIT%
