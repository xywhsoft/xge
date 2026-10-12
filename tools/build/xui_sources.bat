@echo off
set "XUI_SRC="
if "%~1"=="" (
    for /f "delims=" %%S in ('python tools\build\build.py --sources-only') do set "XUI_SRC=%%S"
) else (
    for /f "delims=" %%S in ('python tools\build\build.py "%~1" --sources-only') do set "XUI_SRC=%%S"
)
