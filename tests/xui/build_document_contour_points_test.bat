@echo off
setlocal
cd /d "%~dp0\..\.."
call tools\build\ensure_xge_dll.bat
if %errorlevel% neq 0 exit /b 1
set "PATH=%CD%\build;%PATH%"
call tools\build\build_text_shaping.bat
if %errorlevel% neq 0 exit /b 1
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXGE_TEST_CONTOUR_POINTS @config/includes.rsp tests\xge\test_opentype_shaping.c build\xge.lib -o build\document\xge_contour_points_test.exe
if %errorlevel% neq 0 exit /b 1
build\document\xge_contour_points_test.exe
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXGE_TEST_CONTOUR_POINTS @config/includes.rsp tests\xge\test_rtl_shaping.c build\xge.lib build\text\harfbuzz.o -o build\document\xge_rtl_contour_points_test.exe -lm
if %errorlevel% neq 0 exit /b 1
build\document\xge_rtl_contour_points_test.exe
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXGE_TEST_CONTOUR_POINTS @config/includes.rsp tests\xui\xui_document_opentype_test.c build\xge.lib -o build\document\xui_document_contour_points_test.exe -lm
if %errorlevel% neq 0 exit /b 1
build\document\xui_document_contour_points_test.exe
if %errorlevel% neq 0 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXGE_TEST_CONTOUR_POINTS @config/includes.rsp tests\xui\xui_document_bidi_test.c build\xge.lib -o build\document\xui_document_rtl_contour_points_test.exe -lm
if %errorlevel% neq 0 exit /b 1
build\document\xui_document_rtl_contour_points_test.exe
exit /b %errorlevel%
