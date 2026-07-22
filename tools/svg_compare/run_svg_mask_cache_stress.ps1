param(
	[int]$Iterations = 8,
	[int]$Repeat = 50,
	[string]$OutputDir = "artifacts\svg_mask_cache_stress",
	[switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$exe = Join-Path $root "build\xge_svg.exe"
$outputFull = if ([System.IO.Path]::IsPathRooted($OutputDir)) {
	[System.IO.Path]::GetFullPath($OutputDir)
} else {
	[System.IO.Path]::GetFullPath((Join-Path $root $OutputDir))
}
$cases = @(
	[ordered]@{
		name = "gradient-mask"
		asset = "examples\xge_svg\assets\mask_gradient_batch_33.svg"
	},
	[ordered]@{
		name = "nested-blend-mask"
		asset = "examples\xge_svg\assets\blend_mask_opacity_batch_40.svg"
	},
	[ordered]@{
		name = "clip-mask-use"
		asset = "examples\xge_svg\assets\clip_mask_use.svg"
	},
	[ordered]@{
		name = "scene-mask"
		asset = "examples\xge_svg\assets\mask_shapeex_scene_batch_30.svg"
	}
)

if (($Iterations -le 0) -or ($Repeat -le 0)) {
	throw "Iterations and Repeat must be positive."
}
if (-not $SkipBuild) {
	& (Join-Path $root "build_dll.bat")
	if ($LASTEXITCODE -ne 0) { throw "Failed to build xge.dll." }
	& (Join-Path $root "examples\xge_svg\build.bat")
	if ($LASTEXITCODE -ne 0) { throw "Failed to build xge_svg.exe." }
}
if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) {
	throw "xge_svg.exe not found: $exe"
}
New-Item -ItemType Directory -Force -Path $outputFull | Out-Null

$expectedHashes = @{}
$runs = @()
$failures = @()
Push-Location $root
try {
	for ($iteration = 1; $iteration -le $Iterations; $iteration++) {
		foreach ($item in $cases) {
			$asset = Join-Path $root $item.asset
			$capture = Join-Path $outputFull "$($item.name).png"
			$invocationError = $null
			$averageMs = $null
			$hash = $null
			if (-not (Test-Path -LiteralPath $asset -PathType Leaf)) {
				$invocationError = "asset-missing"
			} else {
				$oldErrorActionPreference = $ErrorActionPreference
				$ErrorActionPreference = "Continue"
				try {
					$output = & $exe --render $asset --capture $capture --width 512 --height 512 --aspect none --repeat $Repeat 2>&1
					$exitCode = $LASTEXITCODE
				} finally {
					$ErrorActionPreference = $oldErrorActionPreference
				}
				if ($exitCode -ne 0) {
					$invocationError = "exit-code:$exitCode"
				} else {
					$line = @($output | Where-Object { $_ -match "render-benchmark" }) | Select-Object -Last 1
					if (($line -eq $null) -or ($line -notmatch "average_ms=([0-9.]+)")) {
						$invocationError = "benchmark-missing"
					} elseif (-not (Test-Path -LiteralPath $capture -PathType Leaf)) {
						$invocationError = "capture-missing"
					} else {
						$averageMs = [double]$matches[1]
						$hash = (Get-FileHash -LiteralPath $capture -Algorithm SHA256).Hash
					}
				}
			}
			$hashMatches = $false
			if ($hash -ne $null) {
				if (-not $expectedHashes.ContainsKey($item.name)) {
					$expectedHashes[$item.name] = $hash
				}
				$hashMatches = $hash -eq $expectedHashes[$item.name]
			}
			if ($invocationError -ne $null) {
				$failures += "$($item.name):${iteration}:$invocationError"
			} elseif (-not $hashMatches) {
				$failures += "$($item.name):${iteration}:hash-drift"
			}
			$runs += [ordered]@{
				case = $item.name
				iteration = $iteration
				average_ms = $averageMs
				capture_sha256 = $hash
				hash_matches = $hashMatches
				invocation_error = $invocationError
				passed = ($invocationError -eq $null) -and $hashMatches
			}
		}
	}
} finally {
	Pop-Location
}

$sourceHashes = @()
foreach ($source in @("src\xge_shape_ex.c") + @($cases | ForEach-Object { $_.asset })) {
	$path = Join-Path $root $source
	$sourceHashes += [ordered]@{
		file = $source
		sha256 = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
	}
}
$expectedRunCount = $Iterations * $cases.Count
$manifest = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	policy = [ordered]@{
		process_lifecycle_per_run = $true
		iterations = $Iterations
		repeat_per_run = $Repeat
		case_count = $cases.Count
		expected_run_count = $expectedRunCount
		stable_capture_hash_required = $true
	}
	expected_hashes = $expectedHashes
	runs = $runs
	source_hashes = $sourceHashes
	failures = $failures
	summary = [ordered]@{
		run_count = $runs.Count
		passed_run_count = @($runs | Where-Object { $_.passed }).Count
		failure_count = $failures.Count
		passed = ($runs.Count -eq $expectedRunCount) -and ($failures.Count -eq 0)
	}
}
$manifestPath = Join-Path $outputFull "manifest_stress.json"
$manifest | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
Write-Host ("SVG mask cache stress: runs={0}/{1}, failures={2}, passed={3}" -f $manifest.summary.passed_run_count, $manifest.summary.run_count, $failures.Count, $manifest.summary.passed)
Write-Host "SVG mask cache stress manifest: $manifestPath"
if (-not $manifest.summary.passed) {
	throw "SVG mask cache stress failed: $($failures -join '; ')"
}
