@echo off
setlocal
call tools\build\ensure_xge_dll.bat
if errorlevel 1 exit /b 1
set OUT_DIR=build\document
if not exist %OUT_DIR% mkdir %OUT_DIR% || exit /b 1
call tools/build/xui_document_sources.bat
gcc -std=c11 -O2 -g -Wall -Wextra -Werror @config/includes.rsp -c tests\xui\xui_test_xrt_impl.c -o %OUT_DIR%\link_owner_fields-xrt.o
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp tests\xui\xui_document_link_owner_fields_test.c %XUI_DOCUMENT_SRC% %OUT_DIR%\link_owner_fields-xrt.o -lm -lws2_32 -liphlpapi -o %OUT_DIR%\xui_document_link_owner_fields_test.exe
if errorlevel 1 exit /b 1
%OUT_DIR%\xui_document_link_owner_fields_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function -DXUI_DOC_REFERENCE_HASH_COLLISION_TEST @config/includes.rsp tests\xui\xui_document_link_owner_fields_test.c %XUI_DOCUMENT_SRC% %OUT_DIR%\link_owner_fields-xrt.o -lm -lws2_32 -liphlpapi -o %OUT_DIR%\xui_document_link_owner_fields_collision_test.exe
if errorlevel 1 exit /b 1
%OUT_DIR%\xui_document_link_owner_fields_collision_test.exe
if errorlevel 1 exit /b 1
gcc -std=c11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function @config/includes.rsp -DXGE_DLL -DXUI_DLL tests\xui\xui_document_link_owner_fields_test.c src\xui\core\xui_unicode.c build\xge.lib -lm -lws2_32 -liphlpapi -o %OUT_DIR%\xui_document_link_owner_fields_dll_test.exe
if errorlevel 1 exit /b 1
set PATH=%CD%\build;%PATH%
%OUT_DIR%\xui_document_link_owner_fields_dll_test.exe
exit /b %errorlevel%
