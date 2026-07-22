param(
	[string]$RendererExe = "artifacts\thorvg_reference_build\tools\svg2png\tvg-svg2png.exe",
	[string]$OutputDir = "artifacts\svg_text_spec_gate",
	[int]$Width = 512,
	[int]$Height = 512,
	[switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

function Assert-ChildPath {
	param(
		[Parameter(Mandatory = $true)][string]$Parent,
		[Parameter(Mandatory = $true)][string]$Child
	)

	$parentFull = [System.IO.Path]::GetFullPath($Parent).TrimEnd(
		[System.IO.Path]::DirectorySeparatorChar,
		[System.IO.Path]::AltDirectorySeparatorChar
	)
	$childFull = [System.IO.Path]::GetFullPath($Child)
	$prefix = $parentFull + [System.IO.Path]::DirectorySeparatorChar
	if (-not $childFull.StartsWith($prefix, [System.StringComparison]::OrdinalIgnoreCase)) {
		throw "Refusing to modify path outside output directory: $childFull"
	}
}

function Get-CaseNames {
	param([Parameter(Mandatory = $true)][object]$Manifest)

	return @($Manifest.cases | ForEach-Object { [string]$_.name })
}

function Add-CaseSetFailures {
	param(
		[Parameter(Mandatory = $true)][string]$Label,
		[Parameter(Mandatory = $true)][string[]]$Expected,
		[Parameter(Mandatory = $true)][string[]]$Actual,
		[Parameter(Mandatory = $true)][ref]$Failures
	)

	$expectedSet = @{}
	$actualSet = @{}
	foreach ($name in $Expected) {
		if ($expectedSet.ContainsKey($name)) { $Failures.Value += "duplicate:expected:$name" }
		else { $expectedSet[$name] = $true }
	}
	foreach ($name in $Actual) {
		if ($actualSet.ContainsKey($name)) { $Failures.Value += "duplicate:$Label`:$name" }
		else { $actualSet[$name] = $true }
	}
	foreach ($name in $expectedSet.Keys) {
		if (-not $actualSet.ContainsKey($name)) { $Failures.Value += "missing:$Label`:$name" }
	}
	foreach ($name in $actualSet.Keys) {
		if (-not $expectedSet.ContainsKey($name)) { $Failures.Value += "extra:$Label`:$name" }
	}
}

function Get-NearColorPixelCount {
	param(
		[Parameter(Mandatory = $true)][string]$Path,
		[Parameter(Mandatory = $true)][int]$Red,
		[Parameter(Mandatory = $true)][int]$Green,
		[Parameter(Mandatory = $true)][int]$Blue,
		[Parameter(Mandatory = $true)][int]$X0,
		[Parameter(Mandatory = $true)][int]$Y0,
		[Parameter(Mandatory = $true)][int]$X1,
		[Parameter(Mandatory = $true)][int]$Y1,
		[int]$Tolerance = 40
	)

	$bitmap = [System.Drawing.Bitmap]::new([string]$Path)
	try {
		$count = 0
		$left = [Math]::Max(0, $X0)
		$top = [Math]::Max(0, $Y0)
		$right = [Math]::Min($bitmap.Width - 1, $X1)
		$bottom = [Math]::Min($bitmap.Height - 1, $Y1)
		for ($y = $top; $y -le $bottom; $y++) {
			for ($x = $left; $x -le $right; $x++) {
				$pixel = $bitmap.GetPixel($x, $y)
				if (([Math]::Abs($pixel.R - $Red) -le $Tolerance) -and
					([Math]::Abs($pixel.G - $Green) -le $Tolerance) -and
					([Math]::Abs($pixel.B - $Blue) -le $Tolerance)) {
					$count++
				}
			}
		}
		return $count
	} finally {
		$bitmap.Dispose()
	}
}

if (($Width -le 0) -or ($Height -le 0)) {
	throw "Width and Height must be positive."
}

Add-Type -AssemblyName System.Drawing
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path
. (Join-Path $scriptDir "svg_compare_cases.ps1")

$outDirFull = if ([System.IO.Path]::IsPathRooted($OutputDir)) {
	[System.IO.Path]::GetFullPath($OutputDir)
} else {
	[System.IO.Path]::GetFullPath((Join-Path $root $OutputDir))
}
$referenceDir = Join-Path $outDirFull "reference"
$normalDir = Join-Path $outDirFull "normal"
$cloneDir = Join-Path $outDirFull "clone"
New-Item -ItemType Directory -Force -Path $outDirFull | Out-Null
foreach ($path in @($referenceDir, $normalDir, $cloneDir)) {
	Assert-ChildPath -Parent $outDirFull -Child $path
	if (Test-Path -LiteralPath $path) { Remove-Item -LiteralPath $path -Recurse -Force }
}

$caseNames = @("text_tspan_inherit")
$catalogByName = @{}
foreach ($case in (Get-XgeSvgCompareCases -IncludeExperimental)) {
	$catalogByName[[string]$case.name] = $case
}
foreach ($name in $caseNames) {
	if (-not $catalogByName.ContainsKey($name)) {
		throw "SVG text specification gate case is missing from the catalog: $name"
	}
}

$referenceParams = @{
	RendererExe = $RendererExe
	OutputDir = $referenceDir
	Width = $Width
	Height = $Height
	ReferenceTag = "thorvg-text-spec-divergence"
	IncludeExperimental = $true
	CaseName = $caseNames
}
& (Join-Path $scriptDir "render_svg_references.ps1") @referenceParams
if ($LASTEXITCODE -ne 0) { throw "ThorVG text specification reference rendering failed." }
$referenceManifestPath = Join-Path $referenceDir ("manifest_thorvg-text-spec-divergence_{0}x{1}.json" -f $Width, $Height)

$compareParams = @{
	OutputDir = $normalDir
	Width = $Width
	Height = $Height
	ReferenceManifest = $referenceManifestPath
	ReferenceTag = "thorvg-text-spec-divergence"
	FailOnReferenceMissing = $true
	IncludeExperimental = $true
	CaseName = $caseNames
	XgePreserveAspectRatio = "none"
}
if ($SkipBuild) { $compareParams["SkipBuild"] = $true }
& (Join-Path $scriptDir "run_svg_compare.ps1") @compareParams
if ($LASTEXITCODE -ne 0) { throw "XGE text specification normal rendering failed." }

$cloneParams = @{} + $compareParams
$cloneParams["OutputDir"] = $cloneDir
$cloneParams["SkipBuild"] = $true
$cloneParams["XgeClone"] = $true
& (Join-Path $scriptDir "run_svg_compare.ps1") @cloneParams
if ($LASTEXITCODE -ne 0) { throw "XGE text specification clone rendering failed." }

$normalManifestPath = Join-Path $normalDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$cloneManifestPath = Join-Path $cloneDir ("manifest_{0}x{1}.json" -f $Width, $Height)
$referenceManifest = Get-Content -LiteralPath $referenceManifestPath -Raw | ConvertFrom-Json
$normalManifest = Get-Content -LiteralPath $normalManifestPath -Raw | ConvertFrom-Json
$cloneManifest = Get-Content -LiteralPath $cloneManifestPath -Raw | ConvertFrom-Json
$failures = @()
Add-CaseSetFailures "reference" $caseNames @(Get-CaseNames $referenceManifest) ([ref]$failures)
Add-CaseSetFailures "normal" $caseNames @(Get-CaseNames $normalManifest) ([ref]$failures)
Add-CaseSetFailures "clone" $caseNames @(Get-CaseNames $cloneManifest) ([ref]$failures)
if (($normalManifest.summary.passed -ne $true) -or ([int]$normalManifest.summary.missing_references -ne 0)) {
	$failures += "failed:normal"
}
if (($cloneManifest.summary.passed -ne $true) -or ([int]$cloneManifest.summary.missing_references -ne 0)) {
	$failures += "failed:clone"
}

$normalEntry = @($normalManifest.cases)[0]
$cloneEntry = @($cloneManifest.cases)[0]
$referenceEntry = @($referenceManifest.cases)[0]
if ([string]$normalEntry.xge_sha256 -cne [string]$cloneEntry.xge_sha256) {
	$failures += "hash-mismatch:normal-clone:text_tspan_inherit"
}

$scaleX = $Width / 512.0
$scaleY = $Height / 512.0
$scaleArea = $scaleX * $scaleY
$regions = @(
	[ordered]@{ name = "A"; r = 56; g = 189; b = 248; x0 = 195; y0 = 130; x1 = 240; y1 = 220; minimum = 100 },
	[ordered]@{ name = "B"; r = 249; g = 115; b = 22; x0 = 238; y0 = 130; x1 = 270; y1 = 220; minimum = 100 },
	[ordered]@{ name = "C"; r = 56; g = 189; b = 248; x0 = 270; y0 = 130; x1 = 315; y1 = 220; minimum = 100 },
	[ordered]@{ name = "D"; r = 167; g = 139; b = 250; x0 = 225; y0 = 350; x1 = 265; y1 = 425; minimum = 100 },
	[ordered]@{ name = "E"; r = 167; g = 139; b = 250; x0 = 265; y0 = 350; x1 = 320; y1 = 425; minimum = 100 }
)
$audits = @()
foreach ($modeEntry in @(
	[ordered]@{ mode = "normal"; png = [string]$normalEntry.xge_png },
	[ordered]@{ mode = "clone"; png = [string]$cloneEntry.xge_png }
)) {
	$counts = [ordered]@{}
	foreach ($region in $regions) {
		$count = Get-NearColorPixelCount -Path $modeEntry.png -Red $region.r -Green $region.g -Blue $region.b `
			-X0 ([Math]::Floor($region.x0 * $scaleX)) -Y0 ([Math]::Floor($region.y0 * $scaleY)) `
			-X1 ([Math]::Ceiling($region.x1 * $scaleX)) -Y1 ([Math]::Ceiling($region.y1 * $scaleY))
		$minimum = [Math]::Max(4, [int][Math]::Floor($region.minimum * $scaleArea))
		$counts[$region.name] = [ordered]@{ pixels = $count; minimum = $minimum; passed = $count -ge $minimum }
		if ($count -lt $minimum) { $failures += "missing-glyph:$($modeEntry.mode):$($region.name)" }
	}
	$audits += [ordered]@{ mode = $modeEntry.mode; png = $modeEntry.png; glyph_regions = $counts }
}

$referenceOrangeCount = Get-NearColorPixelCount -Path ([string]$referenceEntry.reference_png) -Red 249 -Green 115 -Blue 22 `
	-X0 ([Math]::Floor(238 * $scaleX)) -Y0 ([Math]::Floor(130 * $scaleY)) `
	-X1 ([Math]::Ceiling(270 * $scaleX)) -Y1 ([Math]::Ceiling(220 * $scaleY))
$referenceOrangeMaximum = [Math]::Max(2, [int][Math]::Ceiling(8 * $scaleArea))
$referenceDivergencePresent = $referenceOrangeCount -le $referenceOrangeMaximum
if (-not $referenceDivergencePresent) {
	$failures += "thorvg-reference-divergence-resolved:text_tspan_inherit"
}

$result = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	policy = [ordered]@{
		classification = "spec-correct-xge-thorvg-reference-divergence"
		reason = "ThorVG drops leading-space tspan text inherited from xml:space=preserve; XGE retains the SVG text content."
		migration = "Move this case to the regular visual gate when the ThorVG reference renders the B span."
	}
	selected_cases = $caseNames
	reference_manifest = $referenceManifestPath
	normal_manifest = $normalManifestPath
	clone_manifest = $cloneManifestPath
	source_sha256 = (Get-FileHash -LiteralPath (Join-Path $root ([string]$catalogByName["text_tspan_inherit"].svg)) -Algorithm SHA256).Hash.ToLowerInvariant()
	audits = $audits
	reference_audit = [ordered]@{
		png = [string]$referenceEntry.reference_png
		orange_b_region_pixels = $referenceOrangeCount
		maximum_for_known_divergence = $referenceOrangeMaximum
		divergence_present = $referenceDivergencePresent
	}
	failures = $failures
	summary = [ordered]@{
		total_cases = 1
		glyph_region_audits = $regions.Count * 2
		normal_clone_hash_mismatches = if ([string]$normalEntry.xge_sha256 -cne [string]$cloneEntry.xge_sha256) { 1 } else { 0 }
		failure_count = $failures.Count
		passed = $failures.Count -eq 0
	}
}
$manifestPath = Join-Path $outDirFull ("manifest_text_spec_gate_{0}x{1}.json" -f $Width, $Height)
$result | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
Write-Host "SVG text specification gate manifest saved: $manifestPath"
if ($failures.Count -gt 0) {
	throw "SVG text specification gate failed: $($failures -join ', ')"
}
