$ErrorActionPreference = 'Stop'
$peRepository = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
Push-Location -LiteralPath $peRepository
try {
    & '.\examples\xge_particles\build.bat'
    if ($LASTEXITCODE -ne 0) { throw 'Particle example build failed.' }
    foreach ($peName in @('impact', 'fire_smoke', 'explosion', 'weather', 'dust', 'magic', 'confetti')) {
        $peExecutable = Join-Path $peRepository "build\xge_particles_$peName.exe"
        $peOutput = Join-Path $PSScriptRoot "presets\$peName.xson"
        & $peExecutable --export $peOutput
        if ($LASTEXITCODE -ne 0) { throw "Preset export failed: $peName" }
    }
} finally {
    Pop-Location
}
