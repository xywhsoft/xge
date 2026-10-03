@echo off
setlocal
set "OUT_DIR=build\text"
if not "%~1"=="" set "OUT_DIR=%~1"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%" || exit /b 1
set "OBJECT=%OUT_DIR%\harfbuzz.o"
if exist "%OBJECT%" (
    powershell -NoProfile -Command "$stamp=(Get-Item '%OBJECT%').LastWriteTimeUtc; if ((Get-Item 'build_text_shaping.bat').LastWriteTimeUtc -gt $stamp -or (Get-ChildItem 'lib/harfbuzz/src' -Recurse -File | Where-Object { $_.LastWriteTimeUtc -gt $stamp } | Select-Object -First 1)) { exit 1 }"
    if not errorlevel 1 exit /b 0
)
echo [XGE] Building pinned HarfBuzz OpenType shaper...
g++ -std=c++17 -O2 -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -DNDEBUG -DHB_NO_BUFFER_SERIALIZE -DHB_NO_BUFFER_VERIFY -DHB_NO_DRAW -DHB_NO_OT_FETCH -DHB_NO_MMAP -DHB_NO_COLOR -c lib\harfbuzz\src\harfbuzz.cc -o "%OBJECT%"
exit /b %errorlevel%
