# check_all_captures.ps1 — Run pixel assertions for all audit captures
# Usage: powershell -ExecutionPolicy Bypass -File check_all_captures.ps1

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$auditDir = Join-Path $root "artifacts\audit"
$examplesDir = Join-Path $root "examples"

$passCount = 0
$failCount = 0
$skipCount = 0
$failList = @()

Write-Host "============================================"
Write-Host " XGE Visual Audit - Pixel Assertions"
Write-Host "============================================"
Write-Host ""

# Find all audit directories with check_capture.ps1
$auditDirs = Get-ChildItem -Path $examplesDir -Directory -Filter "audit_*" | Sort-Object Name

foreach ($dir in $auditDirs) {
    $checkScript = Join-Path $dir.FullName "check_capture.ps1"
    $captureName = $dir.Name + ".png"
    $capturePath = Join-Path $auditDir $captureName

    if (-not (Test-Path $checkScript)) {
        Write-Host "  [SKIP] $($dir.Name) - no check_capture.ps1"
        $skipCount++
        continue
    }
    if (-not (Test-Path $capturePath)) {
        Write-Host "  [SKIP] $($dir.Name) - no capture PNG"
        $skipCount++
        continue
    }

    Write-Host "  [CHECK] $($dir.Name)..."
    try {
        & powershell -ExecutionPolicy Bypass -File $checkScript -Path $capturePath 2>&1 | Out-Null
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  [OK] $($dir.Name)"
            $passCount++
        } else {
            Write-Host "  [FAIL] $($dir.Name)"
            $failCount++
            $failList += $dir.Name
        }
    } catch {
        Write-Host "  [FAIL] $($dir.Name) - $($_.Exception.Message)"
        $failCount++
        $failList += $dir.Name
    }
}

Write-Host ""
Write-Host "============================================"
Write-Host " RESULTS: $passCount passed, $failCount failed, $skipCount skipped"
if ($failList.Count -gt 0) {
    Write-Host " FAILURES: $($failList -join ', ')"
}
Write-Host "============================================"

if ($failCount -gt 0) { exit 1 }
exit 0
