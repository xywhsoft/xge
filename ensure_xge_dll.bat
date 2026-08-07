@echo off
setlocal

set NEED_BUILD=0
if not exist build\xge.dll set NEED_BUILD=1
if not exist build\xge.lib set NEED_BUILD=1

if %NEED_BUILD% equ 0 (
	powershell -NoProfile -Command "$out=(Get-Item 'build\xge.dll').LastWriteTimeUtc; $paths=@('xge.c','xge.h','xui.h','xge.rc','xui_sources.bat','src','lib'); $newer=Get-ChildItem -Path $paths -Recurse -File | Where-Object { $_.Extension -in '.c','.h','.inl','.rc','.bat' -and $_.LastWriteTimeUtc -gt $out } | Select-Object -First 1; if ($null -ne $newer) { exit 1 }"
	if errorlevel 1 set NEED_BUILD=1
)

if %NEED_BUILD% neq 0 (
	call build_dll.bat
	if errorlevel 1 exit /b %errorlevel%
)

endlocal
exit /b 0
