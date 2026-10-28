@echo off
set "XUI_SRC="
if "%~1"=="" (
    for /f "delims=" %%S in ('python tools\build\build.py --sources-only') do set "XUI_SRC=%%S"
) else (
    for /f "delims=" %%S in ('python tools\build\build.py "%~1" --sources-only') do set "XUI_SRC=%%S"
)
rem Keep the source list compatible with the path substitutions used by the
rem Windows test scripts (for example removing xui_proxy_xge.c).
set "XUI_SRC=%XUI_SRC:/=\%"
