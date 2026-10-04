@echo off
setlocal
call ensure_xge_dll.bat
if errorlevel 1 exit /b 1
set OUT_DIR=build\document
if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1
call xui_document_sources.bat
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -I. -c test_xui\xui_test_xrt_impl.c -o %OUT_DIR%\object_break_origin-xrt.o
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function -I. test_xui\xui_document_object_break_origin_test.c %XUI_DOCUMENT_SRC% %OUT_DIR%\object_break_origin-xrt.o -lm -lws2_32 -liphlpapi -o %OUT_DIR%\xui_document_object_break_origin_test.exe
if errorlevel 1 exit /b 1
%OUT_DIR%\xui_document_object_break_origin_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST -I. test_xui\xui_document_object_break_origin_test.c %XUI_DOCUMENT_SRC% %OUT_DIR%\object_break_origin-xrt.o -lm -lws2_32 -liphlpapi -o %OUT_DIR%\xui_document_object_break_origin_collision_test.exe
if errorlevel 1 exit /b 1
%OUT_DIR%\xui_document_object_break_origin_collision_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function -I. -DXGE_DLL -DXUI_DLL test_xui\xui_document_object_break_origin_test.c src\xui_unicode.c build\xge.lib -lm -lws2_32 -liphlpapi -o %OUT_DIR%\xui_document_object_break_origin_dll_test.exe
if errorlevel 1 exit /b 1
set PATH=%CD%\build;%PATH%
%OUT_DIR%\xui_document_object_break_origin_dll_test.exe
exit /b %errorlevel%
