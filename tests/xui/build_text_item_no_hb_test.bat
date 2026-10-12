@echo off
setlocal
cd /d "%~dp0\..\.."
set "OUT_DIR=build\text-item-no-hb"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
if %errorlevel% neq 0 exit /b 1
call tools/build/xui_sources.bat
rem Test the real native fallback without linking or enabling HarfBuzz.
windres -O coff -i platform\windows\xge.rc -o "%OUT_DIR%\xge_res.o"
if %errorlevel% neq 0 exit /b 1
gcc -shared -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-cast-function-type -DXGE_DLL -DXGE_BUILD_DLL -DXUI_DLL -DXUI_BUILD_DLL -DBUILD_DLL -DXGE_DEBUGMODE=0 @config/includes.rsp "-Wl,--out-implib,%OUT_DIR%\xge.lib" -o "%OUT_DIR%\xge.dll" single\xge.c %XUI_SRC% "%OUT_DIR%\xge_res.o" -lm -lws2_32 -liphlpapi -lgdi32 -luser32 -lshell32 -lole32 -loleaut32 -luuid -limm32 -lwinmm -lavrt
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL @config/includes.rsp tests\xui\xui_text_item_no_hb_test.c "%OUT_DIR%\xge.lib" -o "%OUT_DIR%\xui_text_item_no_hb_test.exe" -lm
if %errorlevel% neq 0 exit /b 1
"%OUT_DIR%\xui_text_item_no_hb_test.exe"
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DTEST_NO_HB @config/includes.rsp tests\xui\xui_document_input_caps_native_test.c "%OUT_DIR%\xge.lib" -o "%OUT_DIR%\xui_document_input_caps_test.exe" -lm
if %errorlevel% neq 0 exit /b 1
"%OUT_DIR%\xui_document_input_caps_test.exe"
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DTEST_NO_HB -DTEST_DRAW_TEXT_ONLY @config/includes.rsp tests\xui\xui_document_input_caps_native_test.c "%OUT_DIR%\xge.lib" -o "%OUT_DIR%\xui_document_input_caps_draw_only_test.exe" -lm
if %errorlevel% neq 0 exit /b 1
"%OUT_DIR%\xui_document_input_caps_draw_only_test.exe"
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DTEST_NO_HB @config/includes.rsp -o "%OUT_DIR%\xui_document_shy_span_spans_test.exe" tests\xui\xui_document_shy_span_native_test.c "%OUT_DIR%\xge.lib" -lm
if %errorlevel% neq 0 exit /b 1
"%OUT_DIR%\xui_document_shy_span_spans_test.exe"
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DTEST_NO_HB -DTEST_DRAW_TEXT_ONLY @config/includes.rsp -o "%OUT_DIR%\xui_document_shy_span_plain_test.exe" tests\xui\xui_document_shy_span_native_test.c "%OUT_DIR%\xge.lib" -lm
if %errorlevel% neq 0 exit /b 1
"%OUT_DIR%\xui_document_shy_span_plain_test.exe"
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DTEST_NO_HB @config/includes.rsp tests\xui\xui_text_range_native_test.c "%OUT_DIR%\xge.lib" -o "%OUT_DIR%\xui_text_range_native_test.exe" -lm
if errorlevel 1 exit /b 1
"%OUT_DIR%\xui_text_range_native_test.exe"
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DTEST_NO_HB -DTEST_BASIC_RANGE @config/includes.rsp -o "%OUT_DIR%\xui_document_shy_span_basic_test.exe" tests\xui\xui_document_shy_span_native_test.c "%OUT_DIR%\xge.lib" -lm
if errorlevel 1 exit /b 1
"%OUT_DIR%\xui_document_shy_span_basic_test.exe"
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DTEST_NO_HB -DTEST_BASIC_RANGE -DTEST_DRAW_TEXT_ONLY @config/includes.rsp -o "%OUT_DIR%\xui_document_shy_span_basic_plain_test.exe" tests\xui\xui_document_shy_span_native_test.c "%OUT_DIR%\xge.lib" -lm
if errorlevel 1 exit /b 1
"%OUT_DIR%\xui_document_shy_span_basic_plain_test.exe"
if errorlevel 1 exit /b 1
exit /b 0
