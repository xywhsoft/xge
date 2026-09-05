@echo off
setlocal
pushd "%~dp0.." || exit /b 1
call build_dll.bat
if errorlevel 1 goto failed
call test\build_particle_test.bat
if errorlevel 1 goto failed
call test\build_particle_dll_test.bat
if errorlevel 1 goto failed
call test\build_particle_render_test.bat
if errorlevel 1 goto failed
call build_test.bat
if errorlevel 1 goto failed
call examples\xge_particles\build.bat
if errorlevel 1 goto failed
if not exist artifacts mkdir artifacts
for %%S in (impact fire_smoke explosion weather dust magic confetti) do (
  for %%F in (json xson) do (
    build\xge_particles_%%S.exe --export artifacts\particles_%%S.%%F
    if errorlevel 1 goto failed
    build\xge_particles_%%S.exe --config artifacts\particles_%%S.%%F --frames 2
    if errorlevel 1 goto failed
  )
)
echo [XGE] Particle suite passed: CPU, DLL, GPU, XGE smoke, seven scenes and JSON/XSON reload.
popd
exit /b 0
:failed
popd
exit /b 1
