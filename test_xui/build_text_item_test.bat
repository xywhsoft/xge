@echo off
setlocal
cd /d "%~dp0\.."
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -I. test_xui\xui_text_item_contract_test.c -o build\xui_text_item_contract_test.exe
if errorlevel 1 exit /b 1
build\xui_text_item_contract_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. test_xui\xui_text_item_native_test.c build\xge.lib -o build\xui_text_item_native_test.exe -lm
if errorlevel 1 exit /b 1
build\xui_text_item_native_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. test_xui\xui_text_range_native_test.c build\xge.lib -o build\xui_text_range_native_test.exe -lm
if errorlevel 1 exit /b 1
build\xui_text_range_native_test.exe
exit /b %errorlevel%
