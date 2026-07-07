param(
	[string]$RendererExe = "",
	[string]$OutputDir = "artifacts\svg_reference_thorvg",
	[int]$Width = 512,
	[int]$Height = 512,
	[string]$ReferenceTag = "thorvg",
	[string]$BackgroundColor = "",
	[switch]$IncludeExperimental,
	[string[]]$CaseName = @(),
	[string[]]$CaseTag = @(),
	[switch]$ListCases,
	[switch]$KeepWorkDir
)

$ErrorActionPreference = "Stop"

if (($Width -le 0) -or ($Height -le 0)) {
	throw "Width and Height must be positive."
}
if ($ReferenceTag -eq "") {
	throw "ReferenceTag must not be empty."
}
if ($PSBoundParameters.ContainsKey("CaseTag")) {
	$effectiveCaseTags = @($CaseTag | Where-Object { ($_ -ne $null) -and ($_ -ne "") })
	if ($effectiveCaseTags.Count -eq 0) {
		throw "SVG compare case tag filter is empty."
	}
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Resolve-Path (Join-Path $scriptDir "..\..")
. (Join-Path $scriptDir "svg_compare_cases.ps1")

$cases = Select-XgeSvgCompareCases -Cases (Get-XgeSvgCompareCases -IncludeExperimental:$IncludeExperimental) -CaseName $CaseName -CaseTag $CaseTag
if ($ListCases) {
	Write-XgeSvgCompareCaseList -Cases $cases
	return
}
if ($RendererExe -eq "") {
	throw "RendererExe is required unless -ListCases is used."
}

function Assert-ChildPath {
	param(
		[string]$Parent,
		[string]$Child
	)

	$parentFull = [System.IO.Path]::GetFullPath($Parent).TrimEnd('\', '/')
	$childFull = [System.IO.Path]::GetFullPath($Child).TrimEnd('\', '/')
	if (-not $childFull.StartsWith($parentFull + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
		throw "Refusing to operate outside output directory: $childFull"
	}
}

$rendererExeFull = $null
if ([System.IO.Path]::IsPathRooted($RendererExe) -or ($RendererExe -match "[\\/]")) {
	$rendererExeFull = if ([System.IO.Path]::IsPathRooted($RendererExe)) { $RendererExe } else { Join-Path $root $RendererExe }
	if (-not (Test-Path $rendererExeFull)) {
		throw "Reference renderer not found: $rendererExeFull"
	}
	$rendererItem = Get-Item -LiteralPath $rendererExeFull
	if ($rendererItem.PSIsContainer) {
		throw "Reference renderer is a directory, expected executable: $rendererExeFull"
	}
} else {
	$rendererCommand = Get-Command $RendererExe -ErrorAction SilentlyContinue
	if ($rendererCommand -eq $null) {
		throw "Reference renderer not found on PATH: $RendererExe"
	}
	$rendererExeFull = $rendererCommand.Source
}

$outDirFull = if ([System.IO.Path]::IsPathRooted($OutputDir)) { $OutputDir } else { Join-Path $root $OutputDir }
New-Item -ItemType Directory -Force -Path $outDirFull | Out-Null

$workDir = Join-Path $outDirFull ("_work_{0}x{1}" -f $Width, $Height)
Assert-ChildPath -Parent $outDirFull -Child $workDir
if (Test-Path $workDir) {
	Remove-Item -LiteralPath $workDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $workDir | Out-Null

try {
	$assetSrc = Join-Path $root "examples\xge_svg\assets"
	$assetDst = Join-Path $workDir "examples\xge_svg\assets"
	New-Item -ItemType Directory -Force -Path (Split-Path -Parent $assetDst) | Out-Null
	Copy-Item -LiteralPath $assetSrc -Destination $assetDst -Recurse

	$results = @()

	foreach ($case in $cases) {
		$tempSvg = Join-Path $workDir $case.svg
		$tempPng = [System.IO.Path]::ChangeExtension($tempSvg, ".png")
		$outPng = Join-Path $outDirFull ("{0}_{1}_{2}x{3}.png" -f $case.name, $ReferenceTag, $Width, $Height)

		if (-not (Test-Path $tempSvg)) {
			throw "SVG case source not found in work dir: $tempSvg"
		}
		if (Test-Path $tempPng) {
			Remove-Item -LiteralPath $tempPng -Force
		}
		if (Test-Path $outPng) {
			Remove-Item -LiteralPath $outPng -Force
		}

		$args = @($tempSvg, "-r", ("{0}x{1}" -f $Width, $Height))
		if ($BackgroundColor -ne "") {
			$args += @("-b", $BackgroundColor)
		}

		& $rendererExeFull @args
		if ($LASTEXITCODE -ne 0) {
			throw "Reference renderer failed for $($case.name)."
		}
		if (-not (Test-Path $tempPng)) {
			throw "Reference renderer did not create expected png: $tempPng"
		}
		Move-Item -LiteralPath $tempPng -Destination $outPng

		$entry = [ordered]@{
			name = $case.name
			svg = $case.svg
			width = $Width
			height = $Height
			tags = @(Get-XgeSvgCompareCaseTags -Case $case)
			reference_png = (Resolve-Path $outPng).Path
			reference_sha256 = (Get-FileHash -Algorithm SHA256 $outPng).Hash.ToLowerInvariant()
		}
		if ($case.Contains("experimental")) {
			$entry["experimental"] = $case.experimental
		}
		$results += $entry
	}

	$manifest = [ordered]@{
		generator = "xge svg reference"
		renderer = (Resolve-Path $rendererExeFull).Path
		reference_tag = $ReferenceTag
		width = $Width
		height = $Height
		case_filters = [ordered]@{
			include_experimental = [bool]$IncludeExperimental
			case_name = @($CaseName)
			case_tag = @($CaseTag)
		}
		background_color = if ($BackgroundColor -ne "") { $BackgroundColor } else { $null }
		summary = [ordered]@{
			total_cases = $results.Count
		}
		cases = $results
	}
	$manifestPath = Join-Path $outDirFull ("manifest_{0}_{1}x{2}.json" -f $ReferenceTag, $Width, $Height)
	$manifest | ConvertTo-Json -Depth 5 | Set-Content -Path $manifestPath -Encoding UTF8
	Write-Host "SVG reference manifest saved: $manifestPath"
} finally {
	if (-not $KeepWorkDir -and (Test-Path $workDir)) {
		Remove-Item -LiteralPath $workDir -Recurse -Force
	}
}
