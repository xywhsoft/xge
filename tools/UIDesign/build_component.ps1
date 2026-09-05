param([string]$Compiler = '', [switch]$Test)
$ErrorActionPreference = 'Stop'
$sdk = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$out = Join-Path $PSScriptRoot 'release/component'
$obj = Join-Path $out 'obj'
if (!$Compiler) {
    $command = Get-Command gcc.exe -ErrorAction SilentlyContinue
    if ($command) { $Compiler = $command.Source }
    elseif (Test-Path -LiteralPath 'E:/software/w64devkit/bin/gcc.exe') { $Compiler = 'E:/software/w64devkit/bin/gcc.exe' }
    else { throw 'Pass -Compiler C:/path/to/gcc.exe (MinGW-w64 64-bit).' }
}
$Compiler = (Resolve-Path -LiteralPath $Compiler).Path
$archive = Join-Path (Split-Path $Compiler) 'ar.exe'
$runtime = Join-Path $sdk 'build/xge.dll'
$import = Join-Path $sdk 'build/xge.lib'
if (!(Test-Path -LiteralPath $runtime) -or !(Test-Path -LiteralPath $import)) {
    throw 'Build the matching XGE/XUI SDK first. This script never overwrites the shared root build.'
}
$inputs = @($runtime, $import, (Join-Path $sdk 'xui.h'), (Join-Path $sdk 'xge.h'))
$before = @($inputs | Get-FileHash -Algorithm SHA256)
$shellSources = @(Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'shell') -Filter '*.c')
if ($shellSources | Select-String -Pattern '#include.*(uid_internal|uid_xui_internal|src/xui)') {
    throw 'The reference host must not use private component/XUI headers.'
}
$componentSources = @(Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'component/src') -Filter '*.c')
if ($componentSources | Select-String -Pattern '\bxge(Init|Run|Begin|End|Unit)\s*\(') {
    throw 'The component must not own a window or frame loop.'
}
New-Item -ItemType Directory -Path $obj -Force | Out-Null
$flags = @('-std=c11','-O0','-g','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-cast-function-type',
    '-Werror=implicit-function-declaration', '-I', $sdk, '-I', (Join-Path $PSScriptRoot 'component/include'))
$objects = @()
foreach ($name in @('uid_core','uid_json','uid_codegen','uid_xui_runtime','uid_editor')) {
    $source = Join-Path $PSScriptRoot "component/src/$name.c"
    $object = Join-Path $obj "$name.o"
    & $Compiler @flags '-c' $source '-o' $object
    if ($LASTEXITCODE) { throw "Compile failed: $name" }
    $objects += $object
}
$library = Join-Path $out 'libuidesign.a'
& $archive rcs $library @objects
if ($LASTEXITCODE) { throw 'Archive failed' }
& $Compiler @flags (Join-Path $PSScriptRoot 'shell/main.c') (Join-Path $PSScriptRoot 'shell/host_files.c') `
    $library $import '-lcomdlg32' '-luser32' '-o' (Join-Path $out 'uidesign_shell.exe')
if ($LASTEXITCODE) { throw 'Shell link failed' }
$testSource = Join-Path $PSScriptRoot 'tests/component_test.c'
if (Test-Path -LiteralPath $testSource) {
    & $Compiler @flags $testSource (Join-Path $sdk 'test_xui/xui_test_proxy.c') $library $import '-o' (Join-Path $out 'component_test.exe')
    if ($LASTEXITCODE) { throw 'Test build failed' }
}
$after = @($inputs | Get-FileHash -Algorithm SHA256)
for ($i = 0; $i -lt $before.Count; $i++) {
    if ($before[$i].Hash -ne $after[$i].Hash) { throw 'Shared SDK changed during build. Retry after the other build finishes.' }
}
Copy-Item -LiteralPath $runtime -Destination (Join-Path $out 'xge.dll') -Force
[IO.File]::WriteAllText((Join-Path $out 'sdk.paths'), ($sdk.Replace('\','/') + "`n" + $Compiler.Replace('\','/') + "`n"), [Text.UTF8Encoding]::new($false))
$after | Select-Object Path,Hash | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $out 'sdk-fingerprint.json') -Encoding UTF8
Write-Host "Built component library and reference shell: $out"
if ($Test) {
    & (Join-Path $out 'component_test.exe') | Tee-Object -FilePath (Join-Path $out 'component-test.log')
    if ($LASTEXITCODE) { throw 'Component tests failed' }
    & (Join-Path $out 'uidesign_shell.exe') --exercise | Tee-Object -FilePath (Join-Path $out 'shell-e2e.log')
    if ($LASTEXITCODE) { throw 'Shell end-to-end test failed' }
}
