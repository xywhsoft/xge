param(
	[string]$ThorvgLoader = "dev\thorvg_reference\src\loaders\svg\tvgSvgLoader.cpp",
	[string]$CoreCoverageManifest = "artifacts\svg_core_coverage_batch147_final\manifest_svg_core_coverage.json",
	[string]$OutputPath = "artifacts\thorvg_svg_loader_parity_batch148\manifest.json"
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path
. (Join-Path $scriptDir "svg_compare_cases.ps1")

function Resolve-RepoPath {
	param([Parameter(Mandatory = $true)][string]$Path)

	if ([System.IO.Path]::IsPathRooted($Path)) {
		return [System.IO.Path]::GetFullPath($Path)
	}
	return [System.IO.Path]::GetFullPath((Join-Path $root $Path))
}

function Get-RelativePath {
	param([Parameter(Mandatory = $true)][string]$Path)

	$full = [System.IO.Path]::GetFullPath($Path)
	if ($full.StartsWith($root, [System.StringComparison]::OrdinalIgnoreCase)) {
		return $full.Substring($root.Length).TrimStart('\')
	}
	return $full
}

function Get-Block {
	param(
		[Parameter(Mandatory = $true)][string]$Text,
		[Parameter(Mandatory = $true)][string]$StartPattern
	)

	$match = [regex]::Match($Text, $StartPattern + '(?<body>[\s\S]*?)\};')
	if (-not $match.Success) {
		throw "ThorVG loader table was not found: $StartPattern"
	}
	return $match.Groups['body'].Value
}

function Get-LineEvidence {
	param(
		[Parameter(Mandatory = $true)][string]$Text,
		[Parameter(Mandatory = $true)][string]$Pattern,
		[Parameter(Mandatory = $true)][string]$Path,
		[int]$Limit = 8
	)

	$items = @()
	$lines = $Text -split "`r?`n"
	for ($i = 0; $i -lt $lines.Count; ++$i) {
		if ([regex]::IsMatch($lines[$i], $Pattern)) {
			$items += [ordered]@{
				file = Get-RelativePath $Path
				line = $i + 1
				text = $lines[$i].Trim()
			}
			if ($items.Count -ge $Limit) { break }
		}
	}
	return $items
}

function New-NameSet {
	param([object[]]$Names)

	$set = @{}
	foreach ($name in @($Names)) {
		$set[[string]$name] = $true
	}
	return $set
}

function Get-CaseMatches {
	param(
		[Parameter(Mandatory = $true)][string]$Pattern,
		[Parameter(Mandatory = $true)][object[]]$Cases,
		[Parameter(Mandatory = $true)][hashtable]$FullNames,
		[Parameter(Mandatory = $true)][hashtable]$PartialNames
	)

	$matches = @()
	foreach ($case in $Cases) {
		$path = Resolve-RepoPath ([string]$case.svg)
		if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { continue }
		$text = Get-Content -LiteralPath $path -Raw
		if (-not [regex]::IsMatch($text, $Pattern, [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)) { continue }
		$name = [string]$case.name
		$ownership = "uncovered"
		if ($FullNames.ContainsKey($name)) {
			$ownership = "full-source"
		} elseif ($PartialNames.ContainsKey($name)) {
			$ownership = "sanitized-partial"
		}
		$matches += [ordered]@{
			name = $name
			file = Get-RelativePath $path
			ownership = $ownership
		}
	}
	return $matches
}

function New-FeatureResult {
	param(
		[Parameter(Mandatory = $true)][string]$Kind,
		[Parameter(Mandatory = $true)][string]$Name,
		[Parameter(Mandatory = $true)][string]$ThorvgPattern,
		[Parameter(Mandatory = $true)][string]$XgePattern,
		[Parameter(Mandatory = $true)][string]$FixturePattern,
		[Parameter(Mandatory = $true)][string]$ThorvgText,
		[Parameter(Mandatory = $true)][string]$ThorvgPath,
		[Parameter(Mandatory = $true)][string]$XgeText,
		[Parameter(Mandatory = $true)][string]$XgePath,
		[Parameter(Mandatory = $true)][object[]]$Cases,
		[Parameter(Mandatory = $true)][hashtable]$FullNames,
		[Parameter(Mandatory = $true)][hashtable]$PartialNames,
		[Parameter(Mandatory = $true)][hashtable]$Deferred
	)

	$thorvgEvidence = @(Get-LineEvidence $ThorvgText $ThorvgPattern $ThorvgPath)
	$xgeEvidence = @(Get-LineEvidence $XgeText $XgePattern $XgePath)
	$caseEvidence = @(Get-CaseMatches $FixturePattern $Cases $FullNames $PartialNames)
	$ownedCases = @($caseEvidence | Where-Object { $_.ownership -ne "uncovered" })
	$status = "required"
	$reason = $null
	$key = "$Kind`:$Name"
	if ($Deferred.ContainsKey($key)) {
		$status = "deferred"
		$reason = [string]$Deferred[$key]
	}
	$passed = $thorvgEvidence.Count -gt 0
	if ($status -eq "required") {
		$passed = $passed -and ($xgeEvidence.Count -gt 0) -and ($ownedCases.Count -gt 0)
	}
	return [ordered]@{
		kind = $Kind
		name = $Name
		status = $status
		deferred_reason = $reason
		thorvg_evidence = $thorvgEvidence
		xge_evidence = $xgeEvidence
		covered_cases = $ownedCases
		uncovered_fixture_cases = @($caseEvidence | Where-Object { $_.ownership -eq "uncovered" })
		passed = $passed
	}
}

$thorvgPath = Resolve-RepoPath $ThorvgLoader
$coveragePath = Resolve-RepoPath $CoreCoverageManifest
$xgePath = Join-Path $root "src\xge_svg.c"
$outputFull = Resolve-RepoPath $OutputPath
foreach ($path in @($thorvgPath, $coveragePath, $xgePath)) {
	if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
		throw "Required parity input is missing: $path"
	}
}

$thorvgText = Get-Content -LiteralPath $thorvgPath -Raw
$xgeText = Get-Content -LiteralPath $xgePath -Raw
$coverage = Get-Content -LiteralPath $coveragePath -Raw | ConvertFrom-Json
$cases = @(Get-XgeSvgCompareCases -IncludeExperimental)
$fullNames = New-NameSet @(
	@($coverage.categories.vector_full_source) +
	@($coverage.categories.image_full_source) +
	@($coverage.categories.root_aspect_full_source) +
	@($coverage.categories.batch156_full_source) +
	@($coverage.categories.text_full_source)
)
$partialNames = New-NameSet @(
	@($coverage.categories.sanitized_nontext_partial) +
	@($coverage.categories.normalized_font_text_partial) +
	@($coverage.categories.spec_correct_text_divergence) +
	@($coverage.categories.allowed_decoder_divergence)
)

$graphicsBlock = Get-Block $thorvgText 'graphicsTags\[\]\s*=\s*\{'
$groupBlock = Get-Block $thorvgText 'groupTags\[\]\s*=\s*\{'
$gradientBlock = Get-Block $thorvgText 'gradientTags\[\]\s*=\s*\{'
$styleBlock = Get-Block $thorvgText 'styleTags\[\]\s*=\s*\{'
$elementNames = @(
	@([regex]::Matches($graphicsBlock, '\{"(?<name>[A-Za-z][A-Za-z0-9]*)"') | ForEach-Object { $_.Groups['name'].Value }) +
	@([regex]::Matches($groupBlock, '\{"(?<name>[A-Za-z][A-Za-z0-9]*)"') | ForEach-Object { $_.Groups['name'].Value }) +
	@([regex]::Matches($gradientBlock, 'GRADIENT_DEF\((?<name>[A-Za-z][A-Za-z0-9]*)\s*,') | ForEach-Object { $_.Groups['name'].Value }) +
	@("stop", "tspan")
) | Sort-Object -Unique
$styleNames = @(
	[regex]::Matches($styleBlock, 'STYLE_DEF\((?<name>[A-Za-z][A-Za-z0-9-]*)\s*,') |
		ForEach-Object { $_.Groups['name'].Value }
) | Sort-Object -Unique

if (($elementNames.Count -lt 20) -or ($styleNames.Count -lt 20)) {
	throw "ThorVG loader extraction returned an implausibly small inventory: elements=$($elementNames.Count), styles=$($styleNames.Count)"
}

$deferred = @{}
$features = @()
foreach ($name in $elementNames) {
	$escaped = [regex]::Escape($name)
	$thorvgPattern = if ($name -eq "stop") {
		'STR_AS\(tagName,\s*"stop"\)'
	} elseif ($name -eq "tspan") {
		'STR_AS\(tagName,\s*"tspan"\)'
	} elseif (($name -eq "linearGradient") -or ($name -eq "radialGradient")) {
		"GRADIENT_DEF\($escaped\s*,"
	} else {
		'\{"' + $escaped + '",\s*sizeof\("' + $escaped + '"\)'
	}
	$xgePattern = '__xgeSvgTagNameEquals\([^\r\n]*"' + $escaped + '"\)'
	$fixturePattern = "<\s*$escaped(?:\s|/|>)"
	$features += New-FeatureResult "element" $name $thorvgPattern $xgePattern $fixturePattern `
		$thorvgText $thorvgPath $xgeText $xgePath $cases $fullNames $partialNames $deferred
}
foreach ($name in $styleNames) {
	$escaped = [regex]::Escape($name)
	$thorvgPattern = "STYLE_DEF\($escaped\s*,"
	$xgePattern = '\{"' + $escaped + '",\s*[01]\}'
	$fixturePattern = "(?<![A-Za-z0-9_-])$escaped\s*(?:=|:)"
	$features += New-FeatureResult "style" $name $thorvgPattern $xgePattern $fixturePattern `
		$thorvgText $thorvgPath $xgeText $xgePath $cases $fullNames $partialNames $deferred
}

$failures = @()
foreach ($feature in $features) {
	if (-not $feature.passed) {
		if ($feature.thorvg_evidence.Count -eq 0) { $failures += "missing-thorvg:$($feature.kind):$($feature.name)" }
		if (($feature.status -eq "required") -and ($feature.xge_evidence.Count -eq 0)) { $failures += "missing-xge:$($feature.kind):$($feature.name)" }
		if (($feature.status -eq "required") -and ($feature.covered_cases.Count -eq 0)) { $failures += "missing-covered-case:$($feature.kind):$($feature.name)" }
	}
}
$deferredFeatures = @($features | Where-Object { $_.status -eq "deferred" })
$requiredFeatures = @($features | Where-Object { $_.status -eq "required" })
$requiredPassed = @($requiredFeatures | Where-Object { $_.passed })
$thorvgCommit = (& git -C (Join-Path $root "dev\thorvg_reference") rev-parse HEAD).Trim()

$manifest = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	thorvg = [ordered]@{
		loader = $thorvgPath
		commit = $thorvgCommit
	}
	xge_source = $xgePath
	core_coverage_manifest = $coveragePath
	policy = [ordered]@{
		text_features_deferred = $false
		image_decoder_differences_allowed = $true
		required_feature_evidence = @("ThorVG loader source", "XGE SVG source", "passing normal/clone coverage ownership")
	}
	features = $features
	failures = $failures
	summary = [ordered]@{
		element_count = $elementNames.Count
		style_property_count = $styleNames.Count
		required_count = $requiredFeatures.Count
		required_passed = $requiredPassed.Count
		deferred_count = $deferredFeatures.Count
		failure_count = $failures.Count
		passed = $failures.Count -eq 0
	}
}
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $outputFull) | Out-Null
$manifest | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $outputFull -Encoding UTF8
Write-Host ("ThorVG SVG loader parity: elements={0}, styles={1}, required={2}/{3}, deferred={4}, failures={5}, passed={6}" -f `
	$elementNames.Count, $styleNames.Count, $requiredPassed.Count, $requiredFeatures.Count, $deferredFeatures.Count, $failures.Count, $manifest.summary.passed)
Write-Host "ThorVG SVG loader parity manifest: $outputFull"
if (-not $manifest.summary.passed) {
	throw "ThorVG SVG loader parity audit failed: $($failures -join '; ')"
}
