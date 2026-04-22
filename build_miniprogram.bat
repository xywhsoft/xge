@echo off
setlocal

set OUT_DIR=dist\miniprogram

if not exist dist (
	mkdir dist || exit /b 1
)
if not exist %OUT_DIR% (
	mkdir %OUT_DIR% || exit /b 1
)
if not exist %OUT_DIR%\platform (
	mkdir %OUT_DIR%\platform || exit /b 1
)
if not exist %OUT_DIR%\examples (
	mkdir %OUT_DIR%\examples || exit /b 1
)
if not exist %OUT_DIR%\examples\miniprogram_hello (
	mkdir %OUT_DIR%\examples\miniprogram_hello || exit /b 1
)

copy /Y platform\miniprogram\xge_miniprogram.js %OUT_DIR%\platform\xge_miniprogram.js >nul || exit /b 1
copy /Y platform\miniprogram\xge_miniprogram_types.d.ts %OUT_DIR%\platform\xge_miniprogram_types.d.ts >nul || exit /b 1
xcopy /E /I /Y examples\miniprogram_hello %OUT_DIR%\examples\miniprogram_hello >nul || exit /b 1

echo [XGE] Mini program scaffold generated: %OUT_DIR%
echo [XGE] Native WASM build is not implemented yet.
endlocal
