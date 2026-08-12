param(
    [Parameter(Mandatory = $true)]
    [string]$Program,

    [Parameter(Mandatory = $true)]
    [string]$Name,

    [string[]]$ArgumentList = @()
)

$ErrorActionPreference = "Stop"
$programPath = (Resolve-Path -LiteralPath $Program).Path
$repoRoot = Split-Path -Parent $PSScriptRoot
$traceDirectory = Join-Path $repoRoot "artifacts\ime_trace"
$tracePath = Join-Path $traceDirectory ($Name + ".log")

New-Item -ItemType Directory -Force -Path $traceDirectory | Out-Null
Remove-Item -LiteralPath $tracePath -Force -ErrorAction SilentlyContinue

$oldTrace = $env:XGE_IME_TRACE
try {
    $env:XGE_IME_TRACE = $tracePath
    $process = Start-Process -FilePath $programPath `
        -WorkingDirectory (Split-Path -Parent $programPath) `
        -ArgumentList $ArgumentList -PassThru
    $process.WaitForExit()
} finally {
    if ($null -eq $oldTrace) {
        Remove-Item Env:XGE_IME_TRACE -ErrorAction SilentlyContinue
    } else {
        $env:XGE_IME_TRACE = $oldTrace
    }
}

if (-not (Test-Path -LiteralPath $tracePath)) {
    throw "IME trace was not created. Verify that the program loaded the instrumented xge.dll."
}

Write-Host "IME trace: $tracePath"
