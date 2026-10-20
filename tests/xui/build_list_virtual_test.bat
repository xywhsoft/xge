@echo off
setlocal
cd /d "%~dp0..\.."
if not defined CC set "CC=gcc"
if not defined XGE_PROFILE set "XGE_PROFILE=build/full-dev"
if not exist "%XGE_PROFILE%\xge.lib" (
  echo Build a matching XGE full-dev profile first.
  exit /b 1
)
"%CC%" -O2 -Wall -Wextra -Wno-unused-parameter -DXGE_DLL -include "%XGE_PROFILE%/xge_build_config.h" @config/includes.rsp tests/xui/xui_list_virtual_test.c tests/xui/xui_test_proxy.c "%XGE_PROFILE%/xge.lib" -lm -luser32 -lshell32 -o "%XGE_PROFILE%/xui_list_virtual_test.exe"
if errorlevel 1 exit /b %errorlevel%
"%XGE_PROFILE%\xui_list_virtual_test.exe"
exit /b %errorlevel%
