@echo off
setlocal

set ROOT=%~dp0

if "%~1"=="/?" goto help
if /i "%~1"=="--help" goto help
if /i "%~1"=="help" goto help

set FRAMES=120
if not "%~1"=="" set FRAMES=%~1

call "%ROOT%examples\core_lifecycle\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\diagnostics_caps_log\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\viewport_clip_camera\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\image_texture_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\font_text_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\resource_provider_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\input_state_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\audio_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\scene_lifecycle_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\render_target_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\shader_variant_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\material_mesh_buffer_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\blend_depth_color_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\shape_full_gallery\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\async_assets_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\platform_runtime_lab\build.bat"
if errorlevel 1 exit /b 1
call "%ROOT%examples\offscreen_egl_lab\build.bat"
if errorlevel 1 exit /b 1

echo [XGE] Running core lifecycle smoke...
"%ROOT%build\xge_core_lifecycle.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running diagnostics caps/log smoke...
"%ROOT%build\xge_diagnostics_caps_log.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running viewport clip camera smoke...
"%ROOT%build\xge_viewport_clip_camera.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running image texture lab smoke...
"%ROOT%build\xge_image_texture_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running font text lab smoke...
"%ROOT%build\xge_font_text_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running resource provider lab smoke...
"%ROOT%build\xge_resource_provider_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running input state lab smoke...
"%ROOT%build\xge_input_state_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running audio lab smoke...
"%ROOT%build\xge_audio_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running scene lifecycle lab smoke...
"%ROOT%build\xge_scene_lifecycle_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running render target lab smoke...
"%ROOT%build\xge_render_target_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running shader variant lab smoke...
"%ROOT%build\xge_shader_variant_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running material mesh buffer lab smoke...
"%ROOT%build\xge_material_mesh_buffer_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running blend depth color lab smoke...
"%ROOT%build\xge_blend_depth_color_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running shape full gallery smoke...
"%ROOT%build\xge_shape_full_gallery.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running async assets lab smoke...
"%ROOT%build\xge_async_assets_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running platform runtime lab smoke...
"%ROOT%build\xge_platform_runtime_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Running offscreen EGL lab smoke...
"%ROOT%build\xge_offscreen_egl_lab.exe" --frames %FRAMES%
if errorlevel 1 exit /b 1

echo [XGE] Example smoke checks finished.
exit /b 0

:help
echo [XGE] Usage: run_examples_smoke.bat [frames]
echo [XGE] Builds and runs examples that support automatic exit.
exit /b 0
