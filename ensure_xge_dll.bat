@echo off
setlocal

set NEED_BUILD=0
if not exist build\xge.dll set NEED_BUILD=1
if not exist build\xge.lib set NEED_BUILD=1

if %NEED_BUILD% equ 0 (
	powershell -NoProfile -Command "$out=(Get-Item 'build\xge.dll').LastWriteTimeUtc; $paths=@('xge.c','xge.h','xui.h','xui_document.h','xui_document_ui.h','xge.rc','build_dll.bat','build_text_shaping.bat','xui_sources.bat','xui_document_sources.bat','src','lib'); $newer=Get-ChildItem -Path $paths -Recurse -File | Where-Object { $_.Extension -in '.c','.cc','.h','.hh','.inl','.inc','.rc','.bat' -and $_.LastWriteTimeUtc -gt $out } | Select-Object -First 1; if ($null -ne $newer) { exit 1 }"
	if errorlevel 1 set NEED_BUILD=1
)

if %NEED_BUILD% neq 0 (
	call build_dll.bat
	if errorlevel 1 exit /b 1
)

rem A local DLL beside a Document test takes precedence over PATH. Refresh
rem existing copies so these tests cannot silently load an earlier build.
if exist build\document\xge.dll (
	copy /Y build\xge.dll build\document\xge.dll >nul
	if errorlevel 1 exit /b 1
)

endlocal
exit /b 0
