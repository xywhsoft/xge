param(
	[string]$OutputDir = "artifacts\svg_compare",
	[int]$Width = 512,
	[int]$Height = 512,
	[string]$ReferenceDir = "",
	[string]$ReferenceTag = "thorvg",
	[switch]$FailOnReferenceMissing,
	[double]$MaxDifferentPixelRatio = -1.0,
	[double]$MaxMeanChannelDiff = -1.0,
	[double]$MaxRmseChannelDiff = -1.0,
	[int]$MaxChannelDiff = -1,
	[switch]$IncludeExperimental
)

$ErrorActionPreference = "Stop"

function Compare-PngPixels {
	param(
		[string]$ActualPath,
		[string]$ReferencePath
	)

	Add-Type -AssemblyName System.Drawing
	$actual = $null
	$reference = $null
	try {
		$actual = New-Object System.Drawing.Bitmap($ActualPath)
		$reference = New-Object System.Drawing.Bitmap($ReferencePath)
		if (($actual.Width -ne $reference.Width) -or ($actual.Height -ne $reference.Height)) {
			return [ordered]@{
				width_equal = $false
				height_equal = $false
				actual_width = $actual.Width
				actual_height = $actual.Height
				reference_width = $reference.Width
				reference_height = $reference.Height
			}
		}

		$totalPixels = [int64]$actual.Width * [int64]$actual.Height
		$differentPixels = [int64]0
		$maxChannelDiff = 0
		$totalChannelDiff = [double]0
		$totalSquaredDiff = [double]0

		for ($y = 0; $y -lt $actual.Height; $y++) {
			for ($x = 0; $x -lt $actual.Width; $x++) {
				$a = $actual.GetPixel($x, $y)
				$r = $reference.GetPixel($x, $y)
				$dr = [Math]::Abs([int]$a.R - [int]$r.R)
				$dg = [Math]::Abs([int]$a.G - [int]$r.G)
				$db = [Math]::Abs([int]$a.B - [int]$r.B)
				$da = [Math]::Abs([int]$a.A - [int]$r.A)
				$pixelDiff = $dr + $dg + $db + $da
				if ($pixelDiff -ne 0) {
					$differentPixels++
				}
				$maxChannelDiff = [Math]::Max($maxChannelDiff, [Math]::Max([Math]::Max($dr, $dg), [Math]::Max($db, $da)))
				$totalChannelDiff += $pixelDiff
				$totalSquaredDiff += ([double]$dr * $dr) + ([double]$dg * $dg) + ([double]$db * $db) + ([double]$da * $da)
			}
		}

		$channelCount = [double]$totalPixels * 4.0
		return [ordered]@{
			width_equal = $true
			height_equal = $true
			width = $actual.Width
			height = $actual.Height
			different_pixels = $differentPixels
			different_pixel_ratio = if ($totalPixels -gt 0) { $differentPixels / [double]$totalPixels } else { 0.0 }
			max_channel_diff = $maxChannelDiff
			mean_channel_diff = if ($channelCount -gt 0.0) { $totalChannelDiff / $channelCount } else { 0.0 }
			rmse_channel_diff = if ($channelCount -gt 0.0) { [Math]::Sqrt($totalSquaredDiff / $channelCount) } else { 0.0 }
		}
	} finally {
		if ($actual -ne $null) {
			$actual.Dispose()
		}
		if ($reference -ne $null) {
			$reference.Dispose()
		}
	}
}

if (($Width -le 0) -or ($Height -le 0)) {
	throw "Width and Height must be positive."
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Resolve-Path (Join-Path $scriptDir "..\..")
. (Join-Path $scriptDir "svg_compare_cases.ps1")

Push-Location $root
	try {
		$exe = Join-Path $root "build\xge_svg.exe"
	if (-not (Test-Path $exe)) {
		& (Join-Path $root "examples\xge_svg\build.bat")
		if ($LASTEXITCODE -ne 0) {
			throw "Failed to build xge_svg.exe."
		}
	}

	$outDirFull = if ([System.IO.Path]::IsPathRooted($OutputDir)) { $OutputDir } else { Join-Path $root $OutputDir }
	New-Item -ItemType Directory -Force -Path $outDirFull | Out-Null
	$refDirFull = $null
	if ($ReferenceDir -ne "") {
		$refDirFull = if ([System.IO.Path]::IsPathRooted($ReferenceDir)) { $ReferenceDir } else { Join-Path $root $ReferenceDir }
		if ($ReferenceTag -eq "") {
			throw "ReferenceTag must not be empty when ReferenceDir is set."
		}
	}

	$cases = Get-XgeSvgCompareCases -IncludeExperimental:$IncludeExperimental
	$results = @()
	$comparedCount = 0
	$missingReferenceCount = 0
	$failedCompareCount = 0

	foreach ($case in $cases) {
		$svgPath = Join-Path $root $case.svg
		$outPng = Join-Path $outDirFull ("{0}_xge_{1}x{2}.png" -f $case.name, $Width, $Height)

		& $exe --render $svgPath --width $Width --height $Height --capture $outPng
		if ($LASTEXITCODE -ne 0) {
			throw "XGE SVG render failed for $($case.name)."
		}

		$entry = [ordered]@{
			name = $case.name
			svg = $case.svg
			width = $Width
			height = $Height
			xge_png = (Resolve-Path $outPng).Path
			xge_sha256 = (Get-FileHash -Algorithm SHA256 $outPng).Hash.ToLowerInvariant()
		}
		if ($case.Contains("experimental")) {
			$entry["experimental"] = $case.experimental
		}

		if ($ReferenceDir -ne "") {
			$refPng = Join-Path $refDirFull ("{0}_{1}_{2}x{3}.png" -f $case.name, $ReferenceTag, $Width, $Height)
			if (Test-Path $refPng) {
				$comparedCount++
				$entry["reference_png"] = (Resolve-Path $refPng).Path
				$entry["reference_sha256"] = (Get-FileHash -Algorithm SHA256 $refPng).Hash.ToLowerInvariant()
				$entry["hash_equal"] = ($entry["xge_sha256"] -eq $entry["reference_sha256"])
				$pixelDiff = Compare-PngPixels -ActualPath $outPng -ReferencePath $refPng
				$thresholdFailures = @()
				if (-not $pixelDiff["width_equal"] -or -not $pixelDiff["height_equal"]) {
					$thresholdFailures += "size"
				} else {
					if (($MaxDifferentPixelRatio -ge 0.0) -and ([double]$pixelDiff["different_pixel_ratio"] -gt $MaxDifferentPixelRatio)) {
						$thresholdFailures += "different_pixel_ratio"
					}
					if (($MaxMeanChannelDiff -ge 0.0) -and ([double]$pixelDiff["mean_channel_diff"] -gt $MaxMeanChannelDiff)) {
						$thresholdFailures += "mean_channel_diff"
					}
					if (($MaxRmseChannelDiff -ge 0.0) -and ([double]$pixelDiff["rmse_channel_diff"] -gt $MaxRmseChannelDiff)) {
						$thresholdFailures += "rmse_channel_diff"
					}
					if (($MaxChannelDiff -ge 0) -and ([int]$pixelDiff["max_channel_diff"] -gt $MaxChannelDiff)) {
						$thresholdFailures += "max_channel_diff"
					}
				}
				$entry["pixel_diff"] = $pixelDiff
				$entry["within_threshold"] = ($thresholdFailures.Count -eq 0)
				if ($thresholdFailures.Count -gt 0) {
					$entry["threshold_failures"] = $thresholdFailures
					$failedCompareCount++
				}
			} else {
				$missingReferenceCount++
				$entry["reference_missing"] = $refPng
				if ($FailOnReferenceMissing) {
					$entry["within_threshold"] = $false
					$entry["threshold_failures"] = @("reference_missing")
					$failedCompareCount++
				}
			}
		}

		$results += $entry
	}

	$manifest = [ordered]@{
		generator = "xge svg compare"
		width = $Width
		height = $Height
		reference_dir = $ReferenceDir
		reference_tag = if ($ReferenceDir -ne "") { $ReferenceTag } else { $null }
		thresholds = [ordered]@{
			fail_on_reference_missing = [bool]$FailOnReferenceMissing
			max_different_pixel_ratio = $MaxDifferentPixelRatio
			max_mean_channel_diff = $MaxMeanChannelDiff
			max_rmse_channel_diff = $MaxRmseChannelDiff
			max_channel_diff = $MaxChannelDiff
		}
		summary = [ordered]@{
			total_cases = $results.Count
			compared_cases = $comparedCount
			missing_references = $missingReferenceCount
			failed_compares = $failedCompareCount
			passed = ($failedCompareCount -eq 0)
		}
		cases = $results
	}
	$manifestPath = Join-Path $outDirFull ("manifest_{0}x{1}.json" -f $Width, $Height)
	$manifest | ConvertTo-Json -Depth 5 | Set-Content -Path $manifestPath -Encoding UTF8
	Write-Host "SVG compare manifest saved: $manifestPath"
	if ($failedCompareCount -gt 0) {
		throw "SVG compare failed: $failedCompareCount case(s) exceeded thresholds or missing references."
	}
} finally {
	Pop-Location
}
