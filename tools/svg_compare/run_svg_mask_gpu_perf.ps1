param(
	[int]$Repeat = 200,
	[int]$Runs = 5,
	[double]$MaxMedianMs = -1.0,
	[string]$OutputDir = "artifacts\svg_mask_gpu_perf",
	[switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path
$exe = Join-Path $root "build\xge_svg.exe"
$assetRelative = "examples\xge_svg\assets\mask_gradient_batch_33.svg"
$asset = Join-Path $root $assetRelative
$outputFull = if ([System.IO.Path]::IsPathRooted($OutputDir)) {
	[System.IO.Path]::GetFullPath($OutputDir)
} else {
	[System.IO.Path]::GetFullPath((Join-Path $root $OutputDir))
}

if (($Repeat -le 0) -or ($Runs -le 0)) {
	throw "Repeat and Runs must be positive."
}
if (-not $SkipBuild) {
	& (Join-Path $root "build_dll.bat")
	if ($LASTEXITCODE -ne 0) { throw "Failed to build xge.dll." }
	& (Join-Path $root "examples\xge_svg\build.bat")
	if ($LASTEXITCODE -ne 0) { throw "Failed to build xge_svg.exe." }
}
if (-not (Test-Path -LiteralPath $exe)) { throw "xge_svg.exe not found: $exe" }
if (-not (Test-Path -LiteralPath $asset)) { throw "Performance asset not found: $asset" }

New-Item -ItemType Directory -Force -Path $outputFull | Out-Null
$capture = Join-Path $outputFull "mask_gradient_batch_33.png"
$samples = @()
Push-Location $root
try {
	for ($run = 1; $run -le $Runs; $run++) {
		$oldErrorActionPreference = $ErrorActionPreference
		$ErrorActionPreference = "Continue"
		try {
			$output = & $exe --render $asset --capture $capture --width 512 --height 512 --aspect none --repeat $Repeat 2>&1
		} finally {
			$ErrorActionPreference = $oldErrorActionPreference
		}
		if ($LASTEXITCODE -ne 0) { throw "Performance run $run failed.`n$($output -join "`n")" }
		$line = @($output | Where-Object { $_ -match "render-benchmark" }) | Select-Object -Last 1
		if (($line -eq $null) -or ($line -notmatch "total_ms=([0-9.]+) average_ms=([0-9.]+)")) {
			throw "Performance run $run did not report timing."
		}
		$samples += [ordered]@{
			run = $run
			total_ms = [double]$matches[1]
			average_ms = [double]$matches[2]
		}
	}
} finally {
	Pop-Location
}

$sorted = @($samples.average_ms | Sort-Object)
if (($sorted.Count % 2) -eq 0) {
	$median = ($sorted[$sorted.Count / 2 - 1] + $sorted[$sorted.Count / 2]) / 2.0
} else {
	$median = $sorted[[int][Math]::Floor($sorted.Count / 2)]
}
$mean = [double](($samples.average_ms | Measure-Object -Average).Average)
$passed = ($MaxMedianMs -lt 0.0) -or ($median -le $MaxMedianMs)
$manifest = [ordered]@{
	generated_utc = [DateTime]::UtcNow.ToString("o")
	asset = $assetRelative
	width = 512
	height = 512
	repeat_per_run = $Repeat
	runs = $Runs
	samples = $samples
	summary = [ordered]@{
		min_average_ms = [double]$sorted[0]
		median_average_ms = [double]$median
		max_average_ms = [double]$sorted[$sorted.Count - 1]
		mean_average_ms = $mean
		max_median_ms = if ($MaxMedianMs -ge 0.0) { $MaxMedianMs } else { $null }
		passed = $passed
	}
}
$manifestPath = Join-Path $outputFull "manifest.json"
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
Write-Host ("SVG mask GPU perf: median={0:F3} ms mean={1:F3} ms min={2:F3} ms max={3:F3} ms" -f $median, $mean, $sorted[0], $sorted[$sorted.Count - 1])
Write-Host "SVG mask GPU perf manifest: $manifestPath"
if (-not $passed) {
	throw ("SVG mask median {0:F3} ms exceeds limit {1:F3} ms." -f $median, $MaxMedianMs)
}
