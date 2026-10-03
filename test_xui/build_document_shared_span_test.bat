@echo off
setlocal
cd /d "%~dp0\.."
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
if not exist build\document mkdir build\document
for %%V in (spans plain) do (
    if "%%V"=="plain" (
        gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DTEST_DRAW_TEXT_ONLY -I. -o build\document\xui_document_shared_span_%%V_test.exe test_xui\xui_document_shared_span_native_test.c build\xge.lib -lm
    ) else (
        gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -I. -o build\document\xui_document_shared_span_%%V_test.exe test_xui\xui_document_shared_span_native_test.c build\xge.lib -lm
    )
    if errorlevel 1 exit /b 1
    set PATH=%CD%\build;%PATH%
    build\document\xui_document_shared_span_%%V_test.exe
    if errorlevel 1 exit /b 1
)
exit /b 0
