param(
	[string]$OutputDir = "artifacts\svg_compare",
	[int]$Width = 512,
	[int]$Height = 512,
	[string]$ReferenceDir = "",
	[string]$ReferenceManifest = "",
	[string]$ReferenceTag = "thorvg",
	[switch]$FailOnReferenceMissing,
	[double]$MaxDifferentPixelRatio = -1.0,
	[double]$MaxMeanChannelDiff = -1.0,
	[double]$MaxRmseChannelDiff = -1.0,
	[int]$MaxChannelDiff = -1,
	[int]$MaxPixelBoundsDelta = -1,
	[int]$AlphaBoundsThreshold = 0,
	[switch]$WriteDiffImages,
	[int]$DiffAmplify = 4,
	[string]$XgePreserveAspectRatio = "",
	[switch]$IncludeExperimental,
	[string[]]$CaseName = @(),
	[string[]]$CaseTag = @(),
	[switch]$SkipBuild,
	[switch]$ListCases
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Resolve-Path (Join-Path $scriptDir "..\..")
$referenceManifestFull = ""
$referenceManifestDir = ""
$referenceManifestData = $null
$referenceCaseByName = @{}

function Test-XgeSourceNewerThan {
	param(
		[System.IO.FileInfo[]]$Sources,
		[string]$TargetPath
	)

	if (-not (Test-Path $TargetPath)) {
		return $true
	}
	$targetTime = (Get-Item $TargetPath).LastWriteTimeUtc
	foreach ($source in $Sources) {
		if ($source.LastWriteTimeUtc -gt $targetTime) {
			return $true
		}
	}
	return $false
}

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
		$differentPixelsGt1 = [int64]0
		$differentPixelsGt4 = [int64]0
		$maxChannelDiff = 0
		$totalChannelDiff = [double]0
		$totalSquaredDiff = [double]0
		$rawDifferentPixels = [int64]0
		$rawMaxChannelDiff = 0
		$rawTotalChannelDiff = [double]0
		$rawTotalSquaredDiff = [double]0

		for ($y = 0; $y -lt $actual.Height; $y++) {
			for ($x = 0; $x -lt $actual.Width; $x++) {
				$a = $actual.GetPixel($x, $y)
				$r = $reference.GetPixel($x, $y)
				$rawDr = [Math]::Abs([int]$a.R - [int]$r.R)
				$rawDg = [Math]::Abs([int]$a.G - [int]$r.G)
				$rawDb = [Math]::Abs([int]$a.B - [int]$r.B)
				$da = [Math]::Abs([int]$a.A - [int]$r.A)
				$aPremulR = [int](([int]$a.R * [int]$a.A + 127) / 255)
				$aPremulG = [int](([int]$a.G * [int]$a.A + 127) / 255)
				$aPremulB = [int](([int]$a.B * [int]$a.A + 127) / 255)
				$rPremulR = [int](([int]$r.R * [int]$r.A + 127) / 255)
				$rPremulG = [int](([int]$r.G * [int]$r.A + 127) / 255)
				$rPremulB = [int](([int]$r.B * [int]$r.A + 127) / 255)
				$dr = [Math]::Abs($aPremulR - $rPremulR)
				$dg = [Math]::Abs($aPremulG - $rPremulG)
				$db = [Math]::Abs($aPremulB - $rPremulB)
				$pixelDiff = $dr + $dg + $db + $da
				$rawPixelDiff = $rawDr + $rawDg + $rawDb + $da
				if ($pixelDiff -ne 0) {
					$differentPixels++
				}
				$pixelMaxDiff = [Math]::Max([Math]::Max($dr, $dg), [Math]::Max($db, $da))
				if ($pixelMaxDiff -gt 1) {
					$differentPixelsGt1++
				}
				if ($pixelMaxDiff -gt 4) {
					$differentPixelsGt4++
				}
				if ($rawPixelDiff -ne 0) {
					$rawDifferentPixels++
				}
				$maxChannelDiff = [Math]::Max($maxChannelDiff, [Math]::Max([Math]::Max($dr, $dg), [Math]::Max($db, $da)))
				$totalChannelDiff += $pixelDiff
				$totalSquaredDiff += ([double]$dr * $dr) + ([double]$dg * $dg) + ([double]$db * $db) + ([double]$da * $da)
				$rawMaxChannelDiff = [Math]::Max($rawMaxChannelDiff, [Math]::Max([Math]::Max($rawDr, $rawDg), [Math]::Max($rawDb, $da)))
				$rawTotalChannelDiff += $rawPixelDiff
				$rawTotalSquaredDiff += ([double]$rawDr * $rawDr) + ([double]$rawDg * $rawDg) + ([double]$rawDb * $rawDb) + ([double]$da * $da)
			}
		}

		$channelCount = [double]$totalPixels * 4.0
		return [ordered]@{
			width_equal = $true
			height_equal = $true
			width = $actual.Width
			height = $actual.Height
			comparison_space = "premultiplied-rgba"
			different_pixels = $differentPixels
			different_pixel_ratio = if ($totalPixels -gt 0) { $differentPixels / [double]$totalPixels } else { 0.0 }
			different_pixels_gt_1 = $differentPixelsGt1
			different_pixel_ratio_gt_1 = if ($totalPixels -gt 0) { $differentPixelsGt1 / [double]$totalPixels } else { 0.0 }
			different_pixels_gt_4 = $differentPixelsGt4
			different_pixel_ratio_gt_4 = if ($totalPixels -gt 0) { $differentPixelsGt4 / [double]$totalPixels } else { 0.0 }
			max_channel_diff = $maxChannelDiff
			mean_channel_diff = if ($channelCount -gt 0.0) { $totalChannelDiff / $channelCount } else { 0.0 }
			rmse_channel_diff = if ($channelCount -gt 0.0) { [Math]::Sqrt($totalSquaredDiff / $channelCount) } else { 0.0 }
			raw_rgba = [ordered]@{
				different_pixels = $rawDifferentPixels
				different_pixel_ratio = if ($totalPixels -gt 0) { $rawDifferentPixels / [double]$totalPixels } else { 0.0 }
				max_channel_diff = $rawMaxChannelDiff
				mean_channel_diff = if ($channelCount -gt 0.0) { $rawTotalChannelDiff / $channelCount } else { 0.0 }
				rmse_channel_diff = if ($channelCount -gt 0.0) { [Math]::Sqrt($rawTotalSquaredDiff / $channelCount) } else { 0.0 }
			}
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

function Write-PngDiffImage {
	param(
		[string]$ActualPath,
		[string]$ReferencePath,
		[string]$OutputPath,
		[int]$Amplify
	)

	Add-Type -AssemblyName System.Drawing
	$actual = $null
	$reference = $null
	$diff = $null
	try {
		$actual = New-Object System.Drawing.Bitmap($ActualPath)
		$reference = New-Object System.Drawing.Bitmap($ReferencePath)
		if (($actual.Width -ne $reference.Width) -or ($actual.Height -ne $reference.Height)) {
			throw "Cannot write diff image for images with different sizes: $ActualPath / $ReferencePath"
		}
		$diff = New-Object System.Drawing.Bitmap($actual.Width, $actual.Height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
		for ($y = 0; $y -lt $actual.Height; $y++) {
			for ($x = 0; $x -lt $actual.Width; $x++) {
				$a = $actual.GetPixel($x, $y)
				$r = $reference.GetPixel($x, $y)
				$aPremulR = [int](([int]$a.R * [int]$a.A + 127) / 255)
				$aPremulG = [int](([int]$a.G * [int]$a.A + 127) / 255)
				$aPremulB = [int](([int]$a.B * [int]$a.A + 127) / 255)
				$rPremulR = [int](([int]$r.R * [int]$r.A + 127) / 255)
				$rPremulG = [int](([int]$r.G * [int]$r.A + 127) / 255)
				$rPremulB = [int](([int]$r.B * [int]$r.A + 127) / 255)
				$dr = [Math]::Abs($aPremulR - $rPremulR)
				$dg = [Math]::Abs($aPremulG - $rPremulG)
				$db = [Math]::Abs($aPremulB - $rPremulB)
				$da = [Math]::Abs([int]$a.A - [int]$r.A)
				if (($dr + $dg + $db + $da) -eq 0) {
					$diff.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(0, 0, 0, 0))
				} elseif (($dr + $dg + $db) -eq 0) {
					$intensity = [Math]::Min(255, [Math]::Max(1, $da * $Amplify))
					$diff.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $intensity, 0, $intensity))
				} else {
					$red = [Math]::Min(255, [Math]::Max(1, $dr * $Amplify))
					$green = [Math]::Min(255, [Math]::Max(1, $dg * $Amplify))
					$blue = [Math]::Min(255, [Math]::Max(1, $db * $Amplify))
					$diff.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $red, $green, $blue))
				}
			}
		}
		$diff.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
	} finally {
		if ($diff -ne $null) {
			$diff.Dispose()
		}
		if ($actual -ne $null) {
			$actual.Dispose()
		}
		if ($reference -ne $null) {
			$reference.Dispose()
		}
	}
}

function Convert-XgeSvgBoundsRect {
	param(
		[string]$Line
	)

	$number = "([-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?)"
	if ($Line -notmatch ("^\s*(local|draw)\s+{0}\s+{0}\s+{0}\s+{0}" -f $number)) {
		return $null
	}
	$culture = [System.Globalization.CultureInfo]::InvariantCulture
	return [ordered]@{
		x = [double]::Parse($matches[2], $culture)
		y = [double]::Parse($matches[3], $culture)
		w = [double]::Parse($matches[4], $culture)
		h = [double]::Parse($matches[5], $culture)
	}
}

function Get-PngAlphaBounds {
	param(
		[string]$Path,
		[int]$Threshold
	)

	Add-Type -AssemblyName System.Drawing
	$bitmap = $null
	try {
		$bitmap = New-Object System.Drawing.Bitmap($Path)
		$minX = $bitmap.Width
		$minY = $bitmap.Height
		$maxX = -1
		$maxY = -1
		$alphaPixels = [int64]0

		for ($y = 0; $y -lt $bitmap.Height; $y++) {
			for ($x = 0; $x -lt $bitmap.Width; $x++) {
				$pixel = $bitmap.GetPixel($x, $y)
				if ([int]$pixel.A -gt $Threshold) {
					$alphaPixels++
					if ($x -lt $minX) { $minX = $x }
					if ($y -lt $minY) { $minY = $y }
					if ($x -gt $maxX) { $maxX = $x }
					if ($y -gt $maxY) { $maxY = $y }
				}
			}
		}

		if ($maxX -lt $minX -or $maxY -lt $minY) {
			return [ordered]@{
				non_empty = $false
				alpha_pixel_count = 0
				x = 0
				y = 0
				w = 0
				h = 0
			}
		}
		return [ordered]@{
			non_empty = $true
			alpha_pixel_count = $alphaPixels
			x = $minX
			y = $minY
			w = $maxX - $minX + 1
			h = $maxY - $minY + 1
		}
	} finally {
		if ($bitmap -ne $null) {
			$bitmap.Dispose()
		}
	}
}

function Compare-PngAlphaBounds {
	param(
		[object]$Actual,
		[object]$Reference
	)

	$actualNonEmpty = [bool]$Actual.non_empty
	$referenceNonEmpty = [bool]$Reference.non_empty
	if (-not $actualNonEmpty -and -not $referenceNonEmpty) {
		return [ordered]@{
			empty_equal = $true
			actual_non_empty = $false
			reference_non_empty = $false
			dx = 0
			dy = 0
			dw = 0
			dh = 0
			max_abs_delta = 0
		}
	}
	if ($actualNonEmpty -ne $referenceNonEmpty) {
		return [ordered]@{
			empty_equal = $false
			actual_non_empty = $actualNonEmpty
			reference_non_empty = $referenceNonEmpty
			dx = $null
			dy = $null
			dw = $null
			dh = $null
			max_abs_delta = $null
		}
	}
	$dx = [int]$Actual.x - [int]$Reference.x
	$dy = [int]$Actual.y - [int]$Reference.y
	$dw = [int]$Actual.w - [int]$Reference.w
	$dh = [int]$Actual.h - [int]$Reference.h
	return [ordered]@{
		empty_equal = $true
		actual_non_empty = $actualNonEmpty
		reference_non_empty = $referenceNonEmpty
		dx = $dx
		dy = $dy
		dw = $dw
		dh = $dh
		max_abs_delta = [Math]::Max([Math]::Max([Math]::Abs($dx), [Math]::Abs($dy)), [Math]::Max([Math]::Abs($dw), [Math]::Abs($dh)))
	}
}

function Read-XgeSvgBounds {
	param(
		[string]$Exe,
		[string]$SvgPath,
		[int]$Width,
		[int]$Height,
		[string]$Aspect
	)

	$args = @("--bounds", $SvgPath, "--width", $Width, "--height", $Height)
	if ($Aspect -ne "") {
		$args += @("--aspect", $Aspect)
	}
	$output = & $Exe @args 2>&1
	if ($LASTEXITCODE -ne 0) {
		throw "XGE SVG bounds failed for ${SvgPath}: $output"
	}
	$local = $null
	$draw = $null
	foreach ($lineObject in $output) {
		$line = [string]$lineObject
		if ($line -match "^\s*local\s+") {
			$local = Convert-XgeSvgBoundsRect -Line $line
		} elseif ($line -match "^\s*draw\s+") {
			$draw = Convert-XgeSvgBoundsRect -Line $line
		}
	}
	if (($local -eq $null) -or ($draw -eq $null)) {
		throw "XGE SVG bounds output could not be parsed for ${SvgPath}: $output"
	}
	return [ordered]@{
		local = $local
		draw = $draw
	}
}

if ($ReferenceManifest -ne "") {
	$referenceManifestFull = if ([System.IO.Path]::IsPathRooted($ReferenceManifest)) { $ReferenceManifest } else { Join-Path $root $ReferenceManifest }
	if (-not (Test-Path $referenceManifestFull)) {
		throw "ReferenceManifest not found: $referenceManifestFull"
	}
	$referenceManifestFull = (Resolve-Path $referenceManifestFull).Path
	$referenceManifestDir = Split-Path -Parent $referenceManifestFull
	$referenceManifestData = Get-Content -LiteralPath $referenceManifestFull -Raw | ConvertFrom-Json

	$referenceTagProperty = $referenceManifestData.PSObject.Properties["reference_tag"]
	if (($referenceTagProperty -ne $null) -and ($referenceTagProperty.Value -ne $null) -and (-not $PSBoundParameters.ContainsKey("ReferenceTag"))) {
		$ReferenceTag = [string]$referenceTagProperty.Value
	}

	$widthProperty = $referenceManifestData.PSObject.Properties["width"]
	if (($widthProperty -ne $null) -and ($widthProperty.Value -ne $null)) {
		$manifestWidth = [int]$widthProperty.Value
		if (-not $PSBoundParameters.ContainsKey("Width")) {
			$Width = $manifestWidth
		} elseif ($manifestWidth -ne $Width) {
			throw "ReferenceManifest width ($manifestWidth) does not match requested Width ($Width)."
		}
	}

	$heightProperty = $referenceManifestData.PSObject.Properties["height"]
	if (($heightProperty -ne $null) -and ($heightProperty.Value -ne $null)) {
		$manifestHeight = [int]$heightProperty.Value
		if (-not $PSBoundParameters.ContainsKey("Height")) {
			$Height = $manifestHeight
		} elseif ($manifestHeight -ne $Height) {
			throw "ReferenceManifest height ($manifestHeight) does not match requested Height ($Height)."
		}
	}

	if ($referenceManifestData.PSObject.Properties["cases"] -eq $null) {
		throw "ReferenceManifest does not contain a cases array: $referenceManifestFull"
	}

	foreach ($refCase in @($referenceManifestData.cases)) {
		if ($refCase -eq $null) {
			continue
		}
		$nameProperty = $refCase.PSObject.Properties["name"]
		$pngProperty = $refCase.PSObject.Properties["reference_png"]
		if (($nameProperty -eq $null) -or ($nameProperty.Value -eq $null) -or ($pngProperty -eq $null) -or ($pngProperty.Value -eq $null)) {
			continue
		}
		$refPng = [string]$pngProperty.Value
		if (-not [System.IO.Path]::IsPathRooted($refPng)) {
			$refPng = Join-Path $referenceManifestDir $refPng
		}
		$referenceCaseByName[[string]$nameProperty.Value] = [ordered]@{
			reference_png = $refPng
			manifest_case = $refCase
		}
	}
}

if (($Width -le 0) -or ($Height -le 0)) {
	throw "Width and Height must be positive."
}
if (($AlphaBoundsThreshold -lt 0) -or ($AlphaBoundsThreshold -gt 255)) {
	throw "AlphaBoundsThreshold must be in the 0..255 range."
}
if ($DiffAmplify -le 0) {
	throw "DiffAmplify must be positive."
}
if ($PSBoundParameters.ContainsKey("CaseTag")) {
	$effectiveCaseTags = @($CaseTag | Where-Object { ($_ -ne $null) -and ($_ -ne "") })
	if ($effectiveCaseTags.Count -eq 0) {
		throw "SVG compare case tag filter is empty."
	}
}

. (Join-Path $scriptDir "svg_compare_cases.ps1")

$cases = Select-XgeSvgCompareCases -Cases (Get-XgeSvgCompareCases -IncludeExperimental:$IncludeExperimental) -CaseName $CaseName -CaseTag $CaseTag
if ($ListCases) {
	Write-XgeSvgCompareCaseList -Cases $cases
	return
}

Push-Location $root
	try {
		$exe = Join-Path $root "build\xge_svg.exe"
		if (-not $SkipBuild) {
			$dll = Join-Path $root "build\xge.dll"
			$lib = Join-Path $root "build\xge.lib"
			$engineSources = @(
				Get-ChildItem -Path $root -File | Where-Object { $_.Extension -in ".c", ".h", ".inl" }
				Get-ChildItem -Path (Join-Path $root "src") -Recurse -File | Where-Object { $_.Extension -in ".c", ".h", ".inl" }
			)
			$dllRebuilt = $false
			if ((Test-XgeSourceNewerThan -Sources $engineSources -TargetPath $dll) -or (-not (Test-Path $lib))) {
				& (Join-Path $root "build_dll.bat")
				if ($LASTEXITCODE -ne 0) {
					throw "Failed to build xge.dll."
				}
				$dllRebuilt = $true
			}
			$exampleSources = @(
				Get-Item (Join-Path $root "examples\xge_svg\main.c")
				Get-Item $lib
			)
			if ($dllRebuilt -or (Test-XgeSourceNewerThan -Sources $exampleSources -TargetPath $exe)) {
				& (Join-Path $root "examples\xge_svg\build.bat")
				if ($LASTEXITCODE -ne 0) {
					throw "Failed to build xge_svg.exe."
				}
			}
		} elseif (-not (Test-Path $exe)) {
			throw "xge_svg.exe is missing while SkipBuild is set."
		}

	$outDirFull = if ([System.IO.Path]::IsPathRooted($OutputDir)) { $OutputDir } else { Join-Path $root $OutputDir }
	New-Item -ItemType Directory -Force -Path $outDirFull | Out-Null
	$refDirFull = $null
	if ($ReferenceDir -ne "") {
		$refDirFull = if ([System.IO.Path]::IsPathRooted($ReferenceDir)) { $ReferenceDir } else { Join-Path $root $ReferenceDir }
	} elseif ($ReferenceManifest -ne "") {
		$refDirFull = $referenceManifestDir
	}
	if (($ReferenceDir -ne "") -or ($ReferenceManifest -ne "")) {
		if ($ReferenceTag -eq "") {
			throw "ReferenceTag must not be empty when a reference source is set."
		}
	}

	$results = @()
	$comparedCount = 0
	$missingReferenceCount = 0
	$failedCompareCount = 0

	foreach ($case in $cases) {
		$svgPath = Join-Path $root $case.svg
		$outPng = Join-Path $outDirFull ("{0}_xge_{1}x{2}.png" -f $case.name, $Width, $Height)
		$bounds = Read-XgeSvgBounds -Exe $exe -SvgPath $svgPath -Width $Width -Height $Height -Aspect $XgePreserveAspectRatio

		$renderArgs = @("--render", $svgPath, "--width", $Width, "--height", $Height, "--capture", $outPng)
		if ($XgePreserveAspectRatio -ne "") {
			$renderArgs += @("--aspect", $XgePreserveAspectRatio)
		}
		& $exe @renderArgs
		if ($LASTEXITCODE -ne 0) {
			throw "XGE SVG render failed for $($case.name)."
		}
		$xgePixelBounds = Get-PngAlphaBounds -Path $outPng -Threshold $AlphaBoundsThreshold

		$entry = [ordered]@{
			name = $case.name
			svg = $case.svg
			width = $Width
			height = $Height
			tags = @(Get-XgeSvgCompareCaseTags -Case $case)
			xge_png = (Resolve-Path $outPng).Path
			xge_sha256 = (Get-FileHash -Algorithm SHA256 $outPng).Hash.ToLowerInvariant()
			xge_bounds = $bounds
			xge_pixel_bounds = $xgePixelBounds
		}
		if ($case.Contains("experimental")) {
			$entry["experimental"] = $case.experimental
		}

		if (($ReferenceDir -ne "") -or ($ReferenceManifest -ne "")) {
			$refPng = $null
			if ($referenceCaseByName.ContainsKey([string]$case.name)) {
				$refPng = [string]$referenceCaseByName[[string]$case.name].reference_png
			} elseif ($refDirFull -ne $null) {
				$refPng = Join-Path $refDirFull ("{0}_{1}_{2}x{3}.png" -f $case.name, $ReferenceTag, $Width, $Height)
			}
			if (Test-Path $refPng) {
				$comparedCount++
				$referencePixelBounds = Get-PngAlphaBounds -Path $refPng -Threshold $AlphaBoundsThreshold
				$pixelBoundsDiff = Compare-PngAlphaBounds -Actual $xgePixelBounds -Reference $referencePixelBounds
				$entry["reference_png"] = (Resolve-Path $refPng).Path
				$entry["reference_sha256"] = (Get-FileHash -Algorithm SHA256 $refPng).Hash.ToLowerInvariant()
				$entry["reference_pixel_bounds"] = $referencePixelBounds
				$entry["pixel_bounds_diff"] = $pixelBoundsDiff
				$entry["hash_equal"] = ($entry["xge_sha256"] -eq $entry["reference_sha256"])
				$pixelDiff = Compare-PngPixels -ActualPath $outPng -ReferencePath $refPng
				if ($WriteDiffImages) {
					$diffPng = Join-Path $outDirFull ("{0}_diff_{1}_{2}x{3}.png" -f $case.name, $ReferenceTag, $Width, $Height)
					Write-PngDiffImage -ActualPath $outPng -ReferencePath $refPng -OutputPath $diffPng -Amplify $DiffAmplify
					$entry["diff_png"] = (Resolve-Path $diffPng).Path
					$entry["diff_amplify"] = $DiffAmplify
				}
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
					if (($MaxPixelBoundsDelta -ge 0) -and ((-not [bool]$pixelBoundsDiff["empty_equal"]) -or ([int]$pixelBoundsDiff["max_abs_delta"] -gt $MaxPixelBoundsDelta))) {
						$thresholdFailures += "pixel_bounds"
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
				$entry["reference_missing"] = if ($refPng -ne $null) { $refPng } else { $case.name }
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
		case_filters = [ordered]@{
			include_experimental = [bool]$IncludeExperimental
			case_name = @($CaseName)
			case_tag = @($CaseTag)
			xge_preserve_aspect_ratio = if ($XgePreserveAspectRatio -ne "") { $XgePreserveAspectRatio } else { $null }
		}
		reference_dir = if ($refDirFull -ne $null) { $refDirFull } else { $ReferenceDir }
		reference_manifest = if ($ReferenceManifest -ne "") { $referenceManifestFull } else { $null }
		reference_tag = if (($ReferenceDir -ne "") -or ($ReferenceManifest -ne "")) { $ReferenceTag } else { $null }
		thresholds = [ordered]@{
			fail_on_reference_missing = [bool]$FailOnReferenceMissing
			max_different_pixel_ratio = $MaxDifferentPixelRatio
			max_mean_channel_diff = $MaxMeanChannelDiff
			max_rmse_channel_diff = $MaxRmseChannelDiff
			max_channel_diff = $MaxChannelDiff
			max_pixel_bounds_delta = $MaxPixelBoundsDelta
			alpha_bounds_threshold = $AlphaBoundsThreshold
			write_diff_images = [bool]$WriteDiffImages
			diff_amplify = if ($WriteDiffImages) { $DiffAmplify } else { $null }
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
