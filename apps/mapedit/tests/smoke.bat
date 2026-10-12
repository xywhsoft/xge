@echo off
setlocal
set TOOL_DIR=%~dp0..
for %%I in ("%TOOL_DIR%") do set TOOL_DIR=%%~fI
set EDITOR=%TOOL_DIR%\build\xge_mapedit_core.exe
if not exist "%EDITOR%" call "%TOOL_DIR%\build.bat" || exit /b 1
"%EDITOR%" --map --frames=7 --smoke-workbench --capture "%TOOL_DIR%\build\workbench.png"
if errorlevel 1 exit /b 1
"%EDITOR%" --map --frames=4 --large-map=1024x1024 --smoke-custom-default
if errorlevel 1 exit /b 1
"%EDITOR%" --tileset --frames=4 --capture "%TOOL_DIR%\build\tileset-font.png"
if errorlevel 1 exit /b 1
echo MapEdit workbench smoke tests passed.
