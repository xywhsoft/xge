@echo off
setlocal
cd /d "%~dp0..\.."
if not defined CC set "CC=gcc"
set "BUILD_DEFAULT_PROFILE=0"
if not defined XGE_PROFILE (
  set "XGE_PROFILE=build/full-dev"
  set "BUILD_DEFAULT_PROFILE=1"
)
if not exist "%XGE_PROFILE%\xge.lib" (
  if "%BUILD_DEFAULT_PROFILE%"=="1" (
    python tools\build\build.py full-dev
    if errorlevel 1 exit /b 1
  ) else (
    echo Build the XGE profile selected by XGE_PROFILE first.
    exit /b 1
  )
)
if not exist "%XGE_PROFILE%\xge.lib" exit /b 1
"%CC%" -O2 -Wall -Wextra -Wno-unused-parameter -DXGE_DLL -include "%XGE_PROFILE%/xge_build_config.h" @config/includes.rsp tests/xui/xui_list_virtual_test.c tests/xui/xui_test_proxy.c "%XGE_PROFILE%/xge.lib" -lm -luser32 -lshell32 -o "%XGE_PROFILE%/xui_list_virtual_test.exe"
if errorlevel 1 exit /b %errorlevel%
"%XGE_PROFILE%\xui_list_virtual_test.exe"
exit /b %errorlevel%
