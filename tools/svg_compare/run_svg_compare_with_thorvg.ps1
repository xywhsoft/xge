param(
	[string]$RendererExe = "",
	[string]$ReferenceDir = "artifacts\svg_reference_thorvg",
	[string]$CompareDir = "artifacts\svg_compare_thorvg",
	[string]$SourceRoot = "",
	[int]$Width = 512,
	[int]$Height = 512,
	[switch]$IncludeExperimental,
	[string[]]$CaseName = @(),
	[string[]]$CaseTag = @(),
	[switch]$ListCases,
	[switch]$SkipReferenceBuild,
	[switch]$BootstrapTools,
	[switch]$CleanThorvgBuild,
	[switch]$KeepReferenceWorkDir,
	[double]$MaxDifferentPixelRatio = -1.0,
	[double]$MaxMeanChannelDiff = -1.0,
	[double]$MaxRmseChannelDiff = -1.0,
	[int]$MaxChannelDiff = -1,
	[int]$MaxPixelBoundsDelta = -1,
	[int]$AlphaBoundsThreshold = 0,
	[double]$VisualScale = 0.25,
	[int]$VisualChannelThreshold = 8,
	[double]$MaxVisualDifferentPixelRatio = -1.0,
	[double]$MaxVisualRmseChannelDiff = -1.0,
	[int]$MaxVisualChannelDiff = -1,
	[switch]$WriteDiffImages,
	[int]$DiffAmplify = 4,
	[string]$XgePreserveAspectRatio = "none"
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path
. (Join-Path $scriptDir "svg_compare_cases.ps1")

if ($ListCases) {
	Write-XgeSvgCompareCaseList -Cases (Select-XgeSvgCompareCases -Cases (Get-XgeSvgCompareCases -IncludeExperimental:$IncludeExperimental) -CaseName $CaseName -CaseTag $CaseTag)
	return
}

function Resolve-XgePath {
	param([string]$Path)

	if ([System.IO.Path]::IsPathRooted($Path)) {
		return [System.IO.Path]::GetFullPath($Path)
	}
	return [System.IO.Path]::GetFullPath((Join-Path $root $Path))
}

if ($RendererExe -eq "") {
	if ($SkipReferenceBuild) {
		throw "RendererExe is required when -SkipReferenceBuild is used."
	}
	$buildParams = @{
		PrintPath = $true
	}
	if ($BootstrapTools) { $buildParams["BootstrapTools"] = $true }
	if ($CleanThorvgBuild) { $buildParams["Clean"] = $true }
	$buildOutput = & (Join-Path $scriptDir "build_thorvg_svg2png.ps1") @buildParams
	if ($LASTEXITCODE -ne 0) {
		throw "Failed to build ThorVG reference renderer."
	}
	$RendererExe = [string](@($buildOutput) | Select-Object -Last 1)
}

$rendererFull = Resolve-XgePath $RendererExe
if (-not (Test-Path $rendererFull)) {
	throw "ThorVG reference renderer not found: $rendererFull"
}

$referenceParams = @{
	RendererExe = $rendererFull
	OutputDir = $ReferenceDir
	SourceRoot = $SourceRoot
	Width = $Width
	Height = $Height
	ReferenceTag = "thorvg"
}
if ($IncludeExperimental) { $referenceParams["IncludeExperimental"] = $true }
if ($CaseName.Count -gt 0) { $referenceParams["CaseName"] = $CaseName }
if ($CaseTag.Count -gt 0) { $referenceParams["CaseTag"] = $CaseTag }
if ($KeepReferenceWorkDir) { $referenceParams["KeepWorkDir"] = $true }

& (Join-Path $scriptDir "render_svg_references.ps1") @referenceParams
if ($LASTEXITCODE -ne 0) {
	throw "Failed to render ThorVG SVG references."
}

$referenceManifest = Join-Path (Resolve-XgePath $ReferenceDir) ("manifest_thorvg_{0}x{1}.json" -f $Width, $Height)
if (-not (Test-Path $referenceManifest)) {
	throw "ThorVG reference manifest was not created: $referenceManifest"
}

$compareParams = @{
	OutputDir = $CompareDir
	SourceRoot = $SourceRoot
	ReferenceManifest = $referenceManifest
	ReferenceTag = "thorvg"
	FailOnReferenceMissing = $true
	Width = $Width
	Height = $Height
	MaxDifferentPixelRatio = $MaxDifferentPixelRatio
	MaxMeanChannelDiff = $MaxMeanChannelDiff
	MaxRmseChannelDiff = $MaxRmseChannelDiff
	MaxChannelDiff = $MaxChannelDiff
	MaxPixelBoundsDelta = $MaxPixelBoundsDelta
	AlphaBoundsThreshold = $AlphaBoundsThreshold
	VisualScale = $VisualScale
	VisualChannelThreshold = $VisualChannelThreshold
	MaxVisualDifferentPixelRatio = $MaxVisualDifferentPixelRatio
	MaxVisualRmseChannelDiff = $MaxVisualRmseChannelDiff
	MaxVisualChannelDiff = $MaxVisualChannelDiff
	DiffAmplify = $DiffAmplify
	XgePreserveAspectRatio = $XgePreserveAspectRatio
}
if ($IncludeExperimental) { $compareParams["IncludeExperimental"] = $true }
if ($CaseName.Count -gt 0) { $compareParams["CaseName"] = $CaseName }
if ($CaseTag.Count -gt 0) { $compareParams["CaseTag"] = $CaseTag }
if ($WriteDiffImages) { $compareParams["WriteDiffImages"] = $true }

& (Join-Path $scriptDir "run_svg_compare.ps1") @compareParams
if ($LASTEXITCODE -ne 0) {
	throw "XGE vs ThorVG SVG compare failed."
}

$compareManifest = Join-Path (Resolve-XgePath $CompareDir) ("manifest_{0}x{1}.json" -f $Width, $Height)
$summary = (Get-Content -LiteralPath $compareManifest -Raw | ConvertFrom-Json).summary
if ([int]$summary.compared_cases -le 0) {
	throw "XGE vs ThorVG SVG compare did not compare any cases."
}
Write-Host "XGE vs ThorVG compare manifest: $compareManifest"
