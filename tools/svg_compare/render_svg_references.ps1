param(
	[string]$RendererExe = "",
	[string]$OutputDir = "artifacts\svg_reference_thorvg",
	[string]$SourceRoot = "",
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
$sourceRootFull = if ($SourceRoot -eq "") {
	$root.Path
} elseif ([System.IO.Path]::IsPathRooted($SourceRoot)) {
	[System.IO.Path]::GetFullPath($SourceRoot)
} else {
	[System.IO.Path]::GetFullPath((Join-Path $root $SourceRoot))
}
if (-not (Test-Path -LiteralPath $sourceRootFull -PathType Container)) {
	throw "SVG source root not found: $sourceRootFull"
}
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
	$assetSrc = Join-Path $sourceRootFull "examples\xge_svg\assets"
	if (-not (Test-Path -LiteralPath $assetSrc -PathType Container)) {
		throw "SVG asset source not found: $assetSrc"
	}
	$assetDst = Join-Path $workDir "examples\xge_svg\assets"
	New-Item -ItemType Directory -Force -Path (Split-Path -Parent $assetDst) | Out-Null
	Copy-Item -LiteralPath $assetSrc -Destination $assetDst -Recurse

	$results = @()

	foreach ($case in $cases) {
		$sourceSvg = [System.IO.Path]::GetFullPath((Join-Path $sourceRootFull ([string]$case.svg)))
		$tempSvg = Join-Path $workDir $case.svg
		$tempPng = [System.IO.Path]::ChangeExtension($tempSvg, ".png")
		$outPng = Join-Path $outDirFull ("{0}_{1}_{2}x{3}.png" -f $case.name, $ReferenceTag, $Width, $Height)

		Assert-ChildPath -Parent $sourceRootFull -Child $sourceSvg
		Assert-ChildPath -Parent $workDir -Child $tempSvg
		if (-not (Test-Path -LiteralPath $sourceSvg -PathType Leaf)) {
			throw "SVG case source not found: $sourceSvg"
		}
		New-Item -ItemType Directory -Force -Path (Split-Path -Parent $tempSvg) | Out-Null
		Copy-Item -LiteralPath $sourceSvg -Destination $tempSvg -Force
		if ($case.Contains("thorvg_reference_wrap_root") -and [bool]$case.thorvg_reference_wrap_root) {
			$sourceDocument = New-Object System.Xml.XmlDocument
			$sourceDocument.PreserveWhitespace = $true
			$sourceDocument.Load($tempSvg)
			if (($sourceDocument.DocumentElement -eq $null) -or ($sourceDocument.DocumentElement.LocalName -cne "svg")) {
				throw "Root-aspect reference source does not have an SVG root: $tempSvg"
			}

			$wrapperDocument = New-Object System.Xml.XmlDocument
			$svgNamespace = "http://www.w3.org/2000/svg"
			$outer = $wrapperDocument.CreateElement("svg", $svgNamespace)
			$outer.SetAttribute("width", [string]$Width)
			$outer.SetAttribute("height", [string]$Height)
			$outer.SetAttribute("viewBox", ("0 0 {0} {1}" -f $Width, $Height))
			$outer.SetAttribute("preserveAspectRatio", "none")
			$defs = $wrapperDocument.CreateElement("defs", $svgNamespace)
			$symbol = $wrapperDocument.CreateElement("symbol", $svgNamespace)
			$symbol.SetAttribute("id", "xge-root-aspect-reference")
			foreach ($attributeName in @("viewBox", "preserveAspectRatio")) {
				$attribute = $sourceDocument.DocumentElement.GetAttributeNode($attributeName)
				if ($attribute -ne $null) {
					$symbol.SetAttribute($attributeName, $attribute.Value)
				}
			}
			foreach ($child in @($sourceDocument.DocumentElement.ChildNodes)) {
				[void]$symbol.AppendChild($wrapperDocument.ImportNode($child, $true))
			}
			[void]$defs.AppendChild($symbol)
			[void]$outer.AppendChild($defs)
			$use = $wrapperDocument.CreateElement("use", $svgNamespace)
			$use.SetAttribute("href", "#xge-root-aspect-reference")
			$use.SetAttribute("x", "0")
			$use.SetAttribute("y", "0")
			$use.SetAttribute("width", [string]$Width)
			$use.SetAttribute("height", [string]$Height)
			[void]$outer.AppendChild($use)
			[void]$wrapperDocument.AppendChild($outer)
			$wrapperDocument.Save($tempSvg)
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
		if ($case.Contains("thorvg_reference_wrap_root")) {
			$entry["thorvg_reference_wrap_root"] = [bool]$case.thorvg_reference_wrap_root
		}
		$results += $entry
	}

	$manifest = [ordered]@{
		generator = "xge svg reference"
		source_root = $sourceRootFull
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
