@echo off
setlocal
cd /d "%~dp0.."
for %%F in (list table tree message timeline) do (
    powershell -NoProfile -ExecutionPolicy Bypass -File test_xui\build_style_collections_test.ps1 -Family %%F
    if errorlevel 1 exit /b 1
)
echo [XUI] All collection color style tests passed.
endlocal
exit /b 0
