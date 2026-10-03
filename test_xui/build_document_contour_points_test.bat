@echo off
setlocal
cd /d "%~dp0\.."
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
if not exist build\document mkdir build\document
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXGE_TEST_CONTOUR_POINTS -I. test\test_opentype_shaping.c build\xge.lib -o build\document\xge_contour_points_test.exe
if errorlevel 1 exit /b 1
build\document\xge_contour_points_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXGE_TEST_CONTOUR_POINTS -I. test\test_rtl_shaping.c build\xge.lib build\text\harfbuzz.o -o build\document\xge_rtl_contour_points_test.exe -lm
if errorlevel 1 exit /b 1
build\document\xge_rtl_contour_points_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXGE_TEST_CONTOUR_POINTS -I. test_xui\xui_document_opentype_test.c build\xge.lib -o build\document\xui_document_contour_points_test.exe -lm
if errorlevel 1 exit /b 1
build\document\xui_document_contour_points_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -DXUI_DLL -DXGE_TEST_CONTOUR_POINTS -I. test_xui\xui_document_bidi_test.c build\xge.lib -o build\document\xui_document_rtl_contour_points_test.exe -lm
if errorlevel 1 exit /b 1
build\document\xui_document_rtl_contour_points_test.exe
exit /b %errorlevel%
