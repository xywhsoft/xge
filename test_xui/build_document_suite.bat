@echo off
setlocal
cd /d "%~dp0\.."
if "%~1"=="" (
    call test_xui\build_document_test.bat
) else (
    call test_xui\build_document_corpus_test.bat "%~1"
)
if errorlevel 1 exit /b 1
call test_xui\build_document_dll_test.bat
if errorlevel 1 exit /b 1
call test_xui\build_document_renderer_test.bat %1
if errorlevel 1 exit /b 1
call test_xui\build_document_editor_test.bat
if errorlevel 1 exit /b 1
call examples\xui_document\build.bat
if errorlevel 1 exit /b 1
build\xui_document.exe --verify
if errorlevel 1 exit /b 1
echo Document core, DLL, renderer, editor and native rendering checks passed.
if "%~1"=="" echo CommonMark corpus was not supplied; its check was skipped.
exit /b 0
