@echo off
setlocal
pushd "%~dp0..\.." || exit /b 1
if not exist build\xge.lib (call build_dll.bat || (popd & exit /b 1))
for %%S in (impact fire_smoke explosion weather dust magic confetti) do (
  echo [XGE] Building particle scene %%S...
  gcc -std=c11 -O2 -Wall -Wextra -Werror -DXGE_DLL -I. examples\xge_particles\common.c examples\xge_particles\%%S.c -o build\xge_particles_%%S.exe build\xge.lib -lm
  if errorlevel 1 (popd & exit /b 1)
)
popd
exit /b 0
