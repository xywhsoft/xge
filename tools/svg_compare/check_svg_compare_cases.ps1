param(
	[int]$MinStableCount = 30,
	[string[]]$RequiredStableTags = @("shape", "path", "clip", "mask", "gradient", "stroke", "pattern", "use")
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Resolve-Path (Join-Path $scriptDir "..\..")
. (Join-Path $scriptDir "svg_compare_cases.ps1")

$stableCases = @(Get-XgeSvgCompareCases)
$allCases = @(Get-XgeSvgCompareCases -IncludeExperimental)
$errors = @()
$seen = @{}

foreach ($case in $allCases) {
	$name = [string]$case.name
	if ($name -eq "") {
		$errors += "SVG compare case has empty name."
		continue
	}
	if ($seen.ContainsKey($name)) {
		$errors += "Duplicate SVG compare case name: $name"
	} else {
		$seen[$name] = $true
	}
	$svgPath = Join-Path $root ([string]$case.svg)
	if (-not (Test-Path -LiteralPath $svgPath)) {
		$errors += "SVG compare asset missing for $name`: $($case.svg)"
	}
}

if ($stableCases.Count -lt $MinStableCount) {
	$errors += "Stable SVG compare case count $($stableCases.Count) is below required minimum $MinStableCount."
}

foreach ($tag in $RequiredStableTags) {
	$matches = @($stableCases | Where-Object { Test-XgeSvgCompareCaseTag -Case $_ -Pattern $tag })
	if ($matches.Count -eq 0) {
		$errors += "Stable SVG compare cases do not cover required tag: $tag"
	}
}

if ($errors.Count -gt 0) {
	foreach ($errorText in $errors) {
		Write-Error $errorText
	}
	exit 1
}

$stableTags = @{}
foreach ($case in $stableCases) {
	foreach ($tag in Get-XgeSvgCompareCaseTags -Case $case) {
		if ($stableTags.ContainsKey($tag)) {
			$stableTags[$tag]++
		} else {
			$stableTags[$tag] = 1
		}
	}
}

Write-Output ("SVG compare cases OK: stable={0}, all={1}" -f $stableCases.Count, $allCases.Count)
Write-Output "Stable tag coverage:"
foreach ($tag in ($stableTags.Keys | Sort-Object)) {
	Write-Output ("  {0}: {1}" -f $tag, $stableTags[$tag])
}
