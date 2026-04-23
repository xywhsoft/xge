@echo off
setlocal

set ROOT=%~dp0..\..
pushd "%ROOT%" || exit /b 1

set OUT_DIR=dist\miniprogram

if not exist dist (
	mkdir dist || (popd && exit /b 1)
)
if not exist "%OUT_DIR%" (
	mkdir "%OUT_DIR%" || (popd && exit /b 1)
)
if not exist "%OUT_DIR%\platform" (
	mkdir "%OUT_DIR%\platform" || (popd && exit /b 1)
)
if not exist "%OUT_DIR%\examples" (
	mkdir "%OUT_DIR%\examples" || (popd && exit /b 1)
)
if not exist "%OUT_DIR%\examples\miniprogram_hello" (
	mkdir "%OUT_DIR%\examples\miniprogram_hello" || (popd && exit /b 1)
)

copy /Y platform\miniprogram\xge_miniprogram.js "%OUT_DIR%\platform\xge_miniprogram.js" >nul || (popd && exit /b 1)
copy /Y platform\miniprogram\xge_miniprogram_types.d.ts "%OUT_DIR%\platform\xge_miniprogram_types.d.ts" >nul || (popd && exit /b 1)
xcopy /E /I /Y examples\miniprogram_hello "%OUT_DIR%\examples\miniprogram_hello" >nul || (popd && exit /b 1)
if exist "%OUT_DIR%\examples\miniprogram_hello\build.bat" del /Q "%OUT_DIR%\examples\miniprogram_hello\build.bat"
if exist "%OUT_DIR%\examples\miniprogram_hello\build.sh" del /Q "%OUT_DIR%\examples\miniprogram_hello\build.sh"

echo [XGE] Mini program scaffold generated: %OUT_DIR%
echo [XGE] Native WASM build is not implemented yet.
popd
endlocal
