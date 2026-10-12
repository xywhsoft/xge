@echo off
set "XUI_DOCUMENT_SRC="
for /f "delims=" %%S in ('python tools\build\build.py --source-group document-core') do set "XUI_DOCUMENT_SRC=%%S"
