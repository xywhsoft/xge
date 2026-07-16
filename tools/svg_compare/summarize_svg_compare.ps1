param(
	[string]$Manifest = "artifacts\svg_compare_thorvg_stable\manifest_512x512.json",
	[string]$CompareDir = "",
	[int]$Width = 512,
	[int]$Height = 512,
	[int]$Top = 20,
	[ValidateSet("diff_ratio", "gt1_ratio", "gt4_ratio", "rmse", "mean", "max", "bounds", "name")]
	[string]$SortBy = "rmse",
	[switch]$FailOnly
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path

if ($CompareDir -ne "") {
	if (-not [System.IO.Path]::IsPathRooted($CompareDir)) {
		$CompareDir = Join-Path $root $CompareDir
	}
	$Manifest = Join-Path $CompareDir ("manifest_{0}x{1}.json" -f $Width, $Height)
}
if (-not [System.IO.Path]::IsPathRooted($Manifest)) {
	$Manifest = Join-Path $root $Manifest
}
if (-not (Test-Path $Manifest)) {
	throw "SVG compare manifest not found: $Manifest"
}

$data = Get-Content -LiteralPath $Manifest -Raw | ConvertFrom-Json
if ($data.PSObject.Properties["summary"] -eq $null) {
	throw "SVG compare manifest has no summary: $Manifest"
}

Write-Host "SVG compare manifest: $Manifest"
$data.summary | Format-List

$cases = @($data.cases | Where-Object { $_ -ne $null })
if ($FailOnly) {
	$cases = @($cases | Where-Object {
		($_.PSObject.Properties["within_threshold"] -ne $null) -and (-not [bool]$_.within_threshold)
	})
}
$cases = @($cases | Where-Object { $_.PSObject.Properties["pixel_diff"] -ne $null })

function Get-SortValue {
	param([object]$Case)

	if ($SortBy -eq "name") { return [string]$Case.name }
	if ($SortBy -eq "rmse") { return [double]$Case.pixel_diff.rmse_channel_diff }
	if ($SortBy -eq "mean") { return [double]$Case.pixel_diff.mean_channel_diff }
	if ($SortBy -eq "max") { return [int]$Case.pixel_diff.max_channel_diff }
	if ($SortBy -eq "gt1_ratio") { return [double]$Case.pixel_diff.different_pixel_ratio_gt_1 }
	if ($SortBy -eq "gt4_ratio") { return [double]$Case.pixel_diff.different_pixel_ratio_gt_4 }
	if ($SortBy -eq "bounds") {
		if ($Case.PSObject.Properties["pixel_bounds_diff"] -eq $null) { return -1 }
		if ($Case.pixel_bounds_diff.max_abs_delta -eq $null) { return [int]::MaxValue }
		return [int]$Case.pixel_bounds_diff.max_abs_delta
	}
	return [double]$Case.pixel_diff.different_pixel_ratio
}

if ($SortBy -eq "name") {
	$cases = @($cases | Sort-Object { Get-SortValue -Case $_ })
} else {
	$cases = @($cases | Sort-Object { Get-SortValue -Case $_ } -Descending)
}

if ($Top -gt 0) {
	$cases = @($cases | Select-Object -First $Top)
}

$cases | Select-Object `
	@{n = "name"; e = { $_.name } },
	@{n = "diff_ratio"; e = { "{0:N6}" -f [double]$_.pixel_diff.different_pixel_ratio } },
	@{n = "gt1_ratio"; e = { "{0:N6}" -f [double]$_.pixel_diff.different_pixel_ratio_gt_1 } },
	@{n = "gt4_ratio"; e = { "{0:N6}" -f [double]$_.pixel_diff.different_pixel_ratio_gt_4 } },
	@{n = "raw_ratio"; e = {
		if ($_.pixel_diff.PSObject.Properties["raw_rgba"] -eq $null) { "" }
		else { "{0:N6}" -f [double]$_.pixel_diff.raw_rgba.different_pixel_ratio }
	} },
	@{n = "mean"; e = { "{0:N3}" -f [double]$_.pixel_diff.mean_channel_diff } },
	@{n = "rmse"; e = { "{0:N3}" -f [double]$_.pixel_diff.rmse_channel_diff } },
	@{n = "max"; e = { $_.pixel_diff.max_channel_diff } },
	@{n = "bounds_delta"; e = {
		if ($_.PSObject.Properties["pixel_bounds_diff"] -eq $null) { "" }
		else { $_.pixel_bounds_diff.max_abs_delta }
	} },
	@{n = "threshold"; e = {
		if ($_.PSObject.Properties["within_threshold"] -eq $null) { "" }
		elseif ([bool]$_.within_threshold) { "ok" }
		else { "fail" }
	} } |
	Format-Table -AutoSize
