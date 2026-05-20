@echo off
setlocal

set ROOT=%~dp0

if "%~1"=="/?" goto help
if /i "%~1"=="--help" goto help
if /i "%~1"=="help" goto help

set MODE=all
if /i "%~1"=="--xui" set MODE=xui
if /i "%~1"=="xui" set MODE=xui

if /i "%MODE%"=="xui" call "%ROOT%build_examples_xui.bat"
if /i "%MODE%"=="xui" exit /b %ERRORLEVEL%
if not "%~1"=="" (
	echo [XGE] Unknown option: %~1
	goto help
)

echo [XGE] Building all Windows examples...
call :do_build "core lifecycle" "examples\core_lifecycle\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "diagnostics caps/log" "examples\diagnostics_caps_log\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "viewport clip camera" "examples\viewport_clip_camera\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "image texture lab" "examples\image_texture_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "font text lab" "examples\font_text_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "resource provider lab" "examples\resource_provider_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "input state lab" "examples\input_state_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "audio lab" "examples\audio_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "scene lifecycle lab" "examples\scene_lifecycle_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "auto pause lab" "examples\auto_pause_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "render target lab" "examples\render_target_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "shader variant lab" "examples\shader_variant_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "material mesh buffer lab" "examples\material_mesh_buffer_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "blend depth color lab" "examples\blend_depth_color_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "shape full gallery" "examples\shape_full_gallery\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "async assets lab" "examples\async_assets_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "platform runtime lab" "examples\platform_runtime_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "offscreen EGL lab" "examples\offscreen_egl_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "render thread lab" "examples\render_thread_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "miniprogram bridge lab" "examples\miniprogram_bridge_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call "%ROOT%build_examples_xui.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "MVP" "examples\mvp\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "app mode" "examples\app_mode\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "async" "examples\async\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "audio" "examples\audio\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "debug overlay" "examples\debug_overlay\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "gamepad" "examples\gamepad\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "input" "examples\input\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "isometric depth" "examples\isometric_depth\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "manual validation" "examples\manual_validation\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "perspective quad" "examples\perspective_quad\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "platform smoke" "examples\platform_smoke\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "render target" "examples\render_target\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "scene" "examples\scene\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "shader distortion" "examples\shader_distortion\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "shader material" "examples\shader_material\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "shape" "examples\shape\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "sprite batch" "examples\sprite_batch\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "text" "examples\text\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "texture" "examples\texture\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "text input" "examples\text_input\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "touch" "examples\touch\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
call :do_build "Game login lab" "examples\game_login_lab\build.bat"
if errorlevel 1 exit /b %ERRORLEVEL%

echo.
echo [XGE] All Windows examples built successfully.
exit /b 0

:do_build
echo.
echo [XGE] Building %~1...
if not exist "%ROOT%%~2" (
	echo [XGE] Missing build script: %~2
	exit /b 1
)
call "%ROOT%%~2"
set CODE=%ERRORLEVEL%
if not "%CODE%"=="0" (
	echo [XGE] Failed: %~1
	exit /b %CODE%
)
exit /b 0

:help
echo [XGE] Usage:
echo [XGE]   build_examples_all.bat
echo [XGE]   build_examples_all.bat --xui
echo [XGE]
echo [XGE] Default builds every Windows/GCC example under examples.
echo [XGE] Platform-specific variants live beside their examples, for example:
echo [XGE]   examples\platform_smoke\build_web.bat
echo [XGE]   examples\platform_smoke\build_linux_x11.sh
exit /b 0
