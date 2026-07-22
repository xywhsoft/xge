param(
	[string]$VectorNormalManifest = "artifacts\svg_compare_visual_batch143_full_normal\manifest_512x512.json",
	[string]$VectorCloneManifest = "artifacts\svg_compare_visual_batch143_full_clone\manifest_512x512.json",
	[string]$NontextManifest = "artifacts\svg_compare_visual_batch143_nontext\manifest_nontext_gate_512x512.json",
	[string]$ImageNormalManifest = "artifacts\svg_image_visual_batch143_normal\manifest_image_gate_512x512.json",
	[string]$ImageCloneManifest = "artifacts\svg_image_visual_batch143_clone\manifest_image_gate_512x512.json",
	[string]$ConvenienceApiManifest = "artifacts\svg_convenience_api_gate_batch144_final\manifest_svg_convenience_api_gate_512x512.json",
	[string]$RootAspectNormalManifest = "artifacts\svg_root_aspect_visual_batch147_normal\manifest_root_aspect_gate_512x512.json",
	[string]$RootAspectCloneManifest = "artifacts\svg_root_aspect_visual_batch147_clone\manifest_root_aspect_gate_512x512.json",
	[string]$Batch156Manifest = "artifacts\svg_batch156_visual_gate_run2\manifest_batch156_visual_gate_512x512.json",
	[string]$TextManifest = "artifacts\svg_compare_visual_text\manifest_text_gate_512x512.json",
	[string]$TextSemanticManifest = "artifacts\svg_compare_text_semantic\manifest_text_semantic_gate_512x512.json",
	[string]$TextSpecManifest = "artifacts\svg_text_spec_gate\manifest_text_spec_gate_512x512.json",
	[string]$DecoderDivergenceManifest = "artifacts\svg_decoder_divergence_gate\manifest_decoder_divergence_gate_512x512.json",
	[string]$OutputPath = "artifacts\svg_core_coverage_batch144\manifest_svg_core_coverage.json"
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

function Read-Manifest {
	param([Parameter(Mandatory = $true)][string]$Path)

	$fullPath = Resolve-RepoPath $Path
	if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
		throw "SVG coverage manifest is missing: $fullPath"
	}
	return [ordered]@{
		path = $fullPath
		data = Get-Content -LiteralPath $fullPath -Raw | ConvertFrom-Json
	}
}

function Get-CaseNames {
	param([Parameter(Mandatory = $true)][object]$Manifest)

	return @($Manifest.cases | ForEach-Object { [string]$_.name })
}

function New-NameSet {
	param([string[]]$Names)

	$set = @{}
	foreach ($name in @($Names)) {
		if (-not $set.ContainsKey($name)) {
			$set[$name] = $true
		}
	}
	return $set
}

function Add-DuplicateFailures {
	param(
		[string]$Label,
		[string[]]$Names,
		[ref]$Failures
	)

	$seen = @{}
	foreach ($name in @($Names)) {
		if ($seen.ContainsKey($name)) {
			$Failures.Value += "duplicate:$Label`:$name"
		} else {
			$seen[$name] = $true
		}
	}
}

function Add-SetDifferenceFailures {
	param(
		[string]$Label,
		[string[]]$Expected,
		[string[]]$Actual,
		[ref]$Failures
	)

	$expectedSet = New-NameSet $Expected
	$actualSet = New-NameSet $Actual
	foreach ($name in $expectedSet.Keys) {
		if (-not $actualSet.ContainsKey($name)) {
			$Failures.Value += "missing:$Label`:$name"
		}
	}
	foreach ($name in $actualSet.Keys) {
		if (-not $expectedSet.ContainsKey($name)) {
			$Failures.Value += "extra:$Label`:$name"
		}
	}
}

function Add-XgeModeFailures {
	param(
		[string]$Label,
		[object]$Manifest,
		[ValidateSet("normal", "clone")][string]$ExpectedMode,
		[ref]$Failures
	)

	$xgeProperty = $Manifest.PSObject.Properties["xge"]
	if (($null -eq $xgeProperty) -or ($null -eq $Manifest.xge)) {
		$Failures.Value += "missing-xge-mode:$Label"
		return
	}
	$modeProperty = $Manifest.xge.PSObject.Properties["mode"]
	$cloneProperty = $Manifest.xge.PSObject.Properties["clone"]
	if ($null -eq $modeProperty) {
		$Failures.Value += "missing-xge-mode:$Label"
	} elseif ([string]$Manifest.xge.mode -cne $ExpectedMode) {
		$Failures.Value += "wrong-xge-mode:$Label`:$($Manifest.xge.mode)"
	}
	if ($null -eq $cloneProperty) {
		$Failures.Value += "missing-xge-clone:$Label"
	} else {
		$expectedClone = $ExpectedMode -ceq "clone"
		if ([bool]$Manifest.xge.clone -ne $expectedClone) {
			$Failures.Value += "wrong-xge-clone:$Label`:$($Manifest.xge.clone)"
		}
	}
}

function Add-GateModeFailures {
	param(
		[string]$Label,
		[object]$Manifest,
		[ValidateSet("normal", "clone")][string]$ExpectedMode,
		[ref]$Failures
	)

	$modeProperty = $Manifest.PSObject.Properties["mode"]
	if ($null -eq $modeProperty) {
		$Failures.Value += "missing-gate-mode:$Label"
	} elseif ([string]$Manifest.mode -cne $ExpectedMode) {
		$Failures.Value += "wrong-gate-mode:$Label`:$($Manifest.mode)"
	}
}

function Add-XgeApiModeFailures {
	param(
		[string]$Label,
		[object]$Manifest,
		[string]$ExpectedMode,
		[string]$ExpectedApi,
		[ref]$Failures
	)

	if (($null -eq $Manifest.xge) -or
	    ([string]$Manifest.xge.mode -cne $ExpectedMode) -or
	    ([string]$Manifest.xge.render_api -cne $ExpectedApi) -or
	    ([bool]$Manifest.xge.clone)) {
		$Failures.Value += "wrong-xge-api-mode:$Label"
	}
}

$vectorNormal = Read-Manifest $VectorNormalManifest
$vectorClone = Read-Manifest $VectorCloneManifest
$nontext = Read-Manifest $NontextManifest
$nontextNormal = Read-Manifest ([string]$nontext.data.normal_manifest)
$nontextClone = Read-Manifest ([string]$nontext.data.clone_manifest)
$imageNormal = Read-Manifest $ImageNormalManifest
$imageClone = Read-Manifest $ImageCloneManifest
$imageNormalStructural = Read-Manifest ([string]$imageNormal.data.structural_manifest)
$imageNormalDecoder = Read-Manifest ([string]$imageNormal.data.decoder_report_manifest)
$imageCloneStructural = Read-Manifest ([string]$imageClone.data.structural_manifest)
$imageCloneDecoder = Read-Manifest ([string]$imageClone.data.decoder_report_manifest)
$convenienceApi = Read-Manifest $ConvenienceApiManifest
$rootAspectNormal = Read-Manifest $RootAspectNormalManifest
$rootAspectClone = Read-Manifest $RootAspectCloneManifest
$rootAspectNormalChild = Read-Manifest ([string]$rootAspectNormal.data.child_manifest)
$rootAspectCloneChild = Read-Manifest ([string]$rootAspectClone.data.child_manifest)
$batch156 = Read-Manifest $Batch156Manifest
$text = Read-Manifest $TextManifest
$textNormal = Read-Manifest ([string]$text.data.normal_manifest)
$textClone = Read-Manifest ([string]$text.data.clone_manifest)
$textSemantic = Read-Manifest $TextSemanticManifest
$textSemanticNormal = Read-Manifest ([string]$textSemantic.data.normal_manifest)
$textSemanticClone = Read-Manifest ([string]$textSemantic.data.clone_manifest)
$textSpec = Read-Manifest $TextSpecManifest
$textSpecNormal = Read-Manifest ([string]$textSpec.data.normal_manifest)
$textSpecClone = Read-Manifest ([string]$textSpec.data.clone_manifest)
$decoderDivergence = Read-Manifest $DecoderDivergenceManifest
$decoderDivergenceNormal = Read-Manifest ([string]$decoderDivergence.data.normal_manifest)
$decoderDivergenceClone = Read-Manifest ([string]$decoderDivergence.data.clone_manifest)
$vectorNormalNames = @(Get-CaseNames $vectorNormal.data)
$vectorCloneNames = @(Get-CaseNames $vectorClone.data)
$nontextNames = @($nontext.data.case_names | ForEach-Object { [string]$_ })
$nontextNormalNames = @(Get-CaseNames $nontextNormal.data)
$nontextCloneNames = @(Get-CaseNames $nontextClone.data)
$imageNormalStructuralNames = @(Get-CaseNames $imageNormalStructural.data)
$imageNormalDecoderNames = @(Get-CaseNames $imageNormalDecoder.data)
$imageCloneStructuralNames = @(Get-CaseNames $imageCloneStructural.data)
$imageCloneDecoderNames = @(Get-CaseNames $imageCloneDecoder.data)
$imageNormalNames = @($imageNormalStructuralNames + $imageNormalDecoderNames)
$imageCloneNames = @($imageCloneStructuralNames + $imageCloneDecoderNames)
$rootAspectNormalNames = @(Get-CaseNames $rootAspectNormalChild.data)
$rootAspectCloneNames = @(Get-CaseNames $rootAspectCloneChild.data)
$batch156Names = @($batch156.data.selected_cases | ForEach-Object { [string]$_ })
$textNames = @($text.data.selected_cases | ForEach-Object { [string]$_ })
$textNormalNames = @(Get-CaseNames $textNormal.data)
$textCloneNames = @(Get-CaseNames $textClone.data)
$textSemanticNames = @($textSemantic.data.selected_cases | ForEach-Object { [string]$_ })
$textSemanticNormalNames = @(Get-CaseNames $textSemanticNormal.data)
$textSemanticCloneNames = @(Get-CaseNames $textSemanticClone.data)
$textSpecNames = @($textSpec.data.selected_cases | ForEach-Object { [string]$_ })
$textSpecNormalNames = @(Get-CaseNames $textSpecNormal.data)
$textSpecCloneNames = @(Get-CaseNames $textSpecClone.data)
$decoderDivergenceNames = @($decoderDivergence.data.selected_cases | ForEach-Object { [string]$_ })
$decoderDivergenceNormalNames = @(Get-CaseNames $decoderDivergenceNormal.data)
$decoderDivergenceCloneNames = @(Get-CaseNames $decoderDivergenceClone.data)
$failures = @()

Add-XgeModeFailures "vector-normal" $vectorNormal.data "normal" ([ref]$failures)
Add-XgeModeFailures "vector-clone" $vectorClone.data "clone" ([ref]$failures)
Add-XgeModeFailures "nontext-normal" $nontextNormal.data "normal" ([ref]$failures)
Add-XgeModeFailures "nontext-clone" $nontextClone.data "clone" ([ref]$failures)
Add-GateModeFailures "image-normal" $imageNormal.data "normal" ([ref]$failures)
Add-GateModeFailures "image-clone" $imageClone.data "clone" ([ref]$failures)
Add-XgeModeFailures "image-normal-structural" $imageNormalStructural.data "normal" ([ref]$failures)
Add-XgeModeFailures "image-normal-decoder" $imageNormalDecoder.data "normal" ([ref]$failures)
Add-XgeModeFailures "image-clone-structural" $imageCloneStructural.data "clone" ([ref]$failures)
Add-XgeModeFailures "image-clone-decoder" $imageCloneDecoder.data "clone" ([ref]$failures)
Add-XgeModeFailures "root-aspect-normal" $rootAspectNormalChild.data "normal" ([ref]$failures)
Add-XgeModeFailures "root-aspect-clone" $rootAspectCloneChild.data "clone" ([ref]$failures)
Add-XgeModeFailures "text-normal" $textNormal.data "normal" ([ref]$failures)
Add-XgeModeFailures "text-clone" $textClone.data "clone" ([ref]$failures)
Add-XgeModeFailures "text-semantic-normal" $textSemanticNormal.data "normal" ([ref]$failures)
Add-XgeModeFailures "text-semantic-clone" $textSemanticClone.data "clone" ([ref]$failures)
Add-XgeModeFailures "text-spec-normal" $textSpecNormal.data "normal" ([ref]$failures)
Add-XgeModeFailures "text-spec-clone" $textSpecClone.data "clone" ([ref]$failures)
Add-XgeModeFailures "decoder-divergence-normal" $decoderDivergenceNormal.data "normal" ([ref]$failures)
Add-XgeModeFailures "decoder-divergence-clone" $decoderDivergenceClone.data "clone" ([ref]$failures)
if ([string]$rootAspectNormal.data.xge_mode -cne "normal") { $failures += "wrong-gate-mode:root-aspect-normal" }
if ([string]$rootAspectClone.data.xge_mode -cne "clone") { $failures += "wrong-gate-mode:root-aspect-clone" }

Add-DuplicateFailures "batch156" $batch156Names ([ref]$failures)
$batch156ExpectedAudits = $batch156Names.Count * 2
if (($batch156.data.summary.passed -ne $true) -or
	([int]$batch156.data.summary.failure_count -ne 0) -or
	([int]$batch156.data.summary.selected_case_count -ne $batch156Names.Count) -or
	([int]$batch156.data.summary.audit_count -ne $batch156ExpectedAudits) -or
	([int]$batch156.data.summary.passed_audit_count -ne $batch156ExpectedAudits) -or
	(@($batch156.data.failures).Count -ne 0)) {
	$failures += "failed:batch156"
}
foreach ($mode in @("normal", "clone")) {
	$actualNames = @($batch156.data.audits |
		Where-Object { [string]$_.mode -ceq $mode } |
		ForEach-Object { [string]$_.name })
	Add-DuplicateFailures "batch156-$mode" $actualNames ([ref]$failures)
	Add-SetDifferenceFailures "batch156-$mode" $batch156Names $actualNames ([ref]$failures)
}

$convenienceExpectedModes = @("direct", "raster-file", "raster-memory", "texture-file", "texture-memory")
$convenienceFileModes = @("direct", "raster-file", "texture-file")
$convenienceSelfNames = @($convenienceApi.data.self_contained_cases | ForEach-Object { [string]$_ })
$convenienceFileNames = @($convenienceApi.data.file_resource_cases | ForEach-Object { [string]$_ })
$convenienceChildren = @{}
foreach ($mode in $convenienceApi.data.modes) {
	$name = [string]$mode.name
	if ($convenienceChildren.ContainsKey($name)) {
		$failures += "duplicate:convenience-mode:$name"
		continue
	}
	$convenienceChildren[$name] = Read-Manifest ([string]$mode.manifest)
}
foreach ($name in $convenienceExpectedModes) {
	if (-not $convenienceChildren.ContainsKey($name)) {
		$failures += "missing:convenience-mode:$name"
		continue
	}
	$child = $convenienceChildren[$name]
	$expectedManifestMode = if ($name -ceq "direct") { "normal" } else { $name }
	Add-XgeApiModeFailures "convenience-$name" $child.data $expectedManifestMode $name ([ref]$failures)
	$expectedNames = @($convenienceSelfNames)
	if ($convenienceFileModes -contains $name) { $expectedNames += $convenienceFileNames }
	$actualNames = @(Get-CaseNames $child.data)
	Add-DuplicateFailures "convenience-$name" $actualNames ([ref]$failures)
	Add-SetDifferenceFailures "convenience-$name" $expectedNames $actualNames ([ref]$failures)
	if (($child.data.summary.passed -ne $true) -or
	    ([int]$child.data.summary.failed_compares -ne 0) -or
	    ([int]$child.data.summary.missing_references -ne 0) -or
	    ([int]$child.data.summary.total_cases -ne $expectedNames.Count) -or
	    ([int]$child.data.summary.compared_cases -ne $expectedNames.Count)) {
		$failures += "failed:convenience-$name"
	}
}
foreach ($name in $convenienceChildren.Keys) {
	if ($convenienceExpectedModes -notcontains $name) {
		$failures += "extra:convenience-mode:$name"
	}
}
$convenienceExpectedComparisons = ($convenienceSelfNames.Count * 5) + ($convenienceFileNames.Count * 3)
if (($convenienceApi.data.summary.passed -ne $true) -or
	([int]$convenienceApi.data.summary.cross_mode_hash_failures -ne 0) -or
	([int]$convenienceApi.data.summary.mode_count -ne 5) -or
	([int]$convenienceApi.data.summary.self_contained_case_count -ne $convenienceSelfNames.Count) -or
	([int]$convenienceApi.data.summary.file_resource_case_count -ne $convenienceFileNames.Count) -or
	([int]$convenienceApi.data.summary.comparison_count -ne $convenienceExpectedComparisons)) {
	$failures += "failed:convenience-api"
}
if ($convenienceChildren.ContainsKey("direct")) {
	$baselineHashes = @{}
	foreach ($entry in $convenienceChildren["direct"].data.cases) {
		$baselineHashes[[string]$entry.name] = [string]$entry.xge_sha256
	}
	foreach ($modeName in ($convenienceExpectedModes | Where-Object { $_ -cne "direct" })) {
		if (-not $convenienceChildren.ContainsKey($modeName)) { continue }
		foreach ($entry in $convenienceChildren[$modeName].data.cases) {
			$name = [string]$entry.name
			if ((-not $baselineHashes.ContainsKey($name)) -or
			    ($baselineHashes[$name] -cne [string]$entry.xge_sha256)) {
				$failures += "hash-mismatch:convenience-$modeName`:$name"
			}
		}
	}
}

foreach ($entry in @(
	@("vector-normal", $vectorNormal.data.summary),
	@("vector-clone", $vectorClone.data.summary),
	@("text-normal", $textNormal.data.summary),
	@("text-clone", $textClone.data.summary),
	@("text-semantic-normal", $textSemanticNormal.data.summary),
	@("text-semantic-clone", $textSemanticClone.data.summary),
	@("text-spec-normal", $textSpecNormal.data.summary),
	@("text-spec-clone", $textSpecClone.data.summary),
	@("decoder-divergence-normal", $decoderDivergenceNormal.data.summary),
	@("decoder-divergence-clone", $decoderDivergenceClone.data.summary)
)) {
	$label = [string]$entry[0]
	$summary = $entry[1]
	if (($summary.passed -ne $true) -or ([int]$summary.failed_compares -ne 0) -or
	    ([int]$summary.missing_references -ne 0)) {
		$failures += "failed:$label"
	}
}
if (($nontext.data.summary.passed -ne $true) -or
	([int]$nontext.data.summary.normal_failed_cases -ne 0) -or
	([int]$nontext.data.summary.clone_failed_cases -ne 0)) {
	$failures += "failed:nontext"
}
if (($text.data.summary.passed -ne $true) -or
	([int]$text.data.summary.normal_failed_cases -ne 0) -or
	([int]$text.data.summary.clone_failed_cases -ne 0) -or
	([int]$text.data.summary.normal_clone_hash_mismatches -ne 0) -or
	([int]$text.data.summary.failure_count -ne 0) -or
	(@($text.data.failures).Count -ne 0)) {
	$failures += "failed:text"
}
if (($textSemantic.data.summary.passed -ne $true) -or
	([int]$textSemantic.data.summary.normal_failed_cases -ne 0) -or
	([int]$textSemantic.data.summary.clone_failed_cases -ne 0) -or
	([int]$textSemantic.data.summary.normal_clone_hash_mismatches -ne 0) -or
	([int]$textSemantic.data.summary.failure_count -ne 0) -or
	(@($textSemantic.data.failures).Count -ne 0)) {
	$failures += "failed:text-semantic"
}
$textSpecExpected = @("text_tspan_inherit")
Add-SetDifferenceFailures "text-spec-wrapper" $textSpecExpected $textSpecNames ([ref]$failures)
if (($textSpec.data.summary.passed -ne $true) -or
	([int]$textSpec.data.summary.total_cases -ne $textSpecExpected.Count) -or
	([int]$textSpec.data.summary.glyph_region_audits -ne 10) -or
	([int]$textSpec.data.summary.normal_clone_hash_mismatches -ne 0) -or
	([int]$textSpec.data.summary.failure_count -ne 0) -or
	([string]$textSpec.data.policy.classification -cne "spec-correct-xge-thorvg-reference-divergence") -or
	($textSpec.data.reference_audit.divergence_present -ne $true) -or
	(@($textSpec.data.failures).Count -ne 0)) {
	$failures += "failed:text-spec"
}
$decoderDivergenceExpected = @("mask_external_scene_batch_31", "mask_luminance_batch_32")
Add-SetDifferenceFailures "decoder-divergence-wrapper" $decoderDivergenceExpected $decoderDivergenceNames ([ref]$failures)
if (($decoderDivergence.data.summary.passed -ne $true) -or
	([int]$decoderDivergence.data.summary.total_cases -ne $decoderDivergenceExpected.Count) -or
	([int]$decoderDivergence.data.summary.audited_cases -ne $decoderDivergenceExpected.Count) -or
	([int]$decoderDivergence.data.summary.normal_clone_hash_mismatches -ne 0) -or
	([int]$decoderDivergence.data.summary.failure_count -ne 0) -or
	([string]$decoderDivergence.data.policy.classification -cne "allowed-image-decoder-divergence") -or
	(@($decoderDivergence.data.failures).Count -ne 0) -or
	(@($decoderDivergence.data.audits | Where-Object {
		($_.normal_clone_hash_equal -ne $true) -or ($_.reference_divergence_present -ne $true)
	}).Count -ne 0)) {
	$failures += "failed:decoder-divergence"
}
foreach ($entry in @(
	@("nontext-normal", $nontextNormal.data.summary),
	@("nontext-clone", $nontextClone.data.summary)
)) {
	$label = [string]$entry[0]
	$summary = $entry[1]
	if (($summary.passed -ne $true) -or ([int]$summary.failed_compares -ne 0) -or
	    ([int]$summary.missing_references -ne 0)) {
		$failures += "failed:$label"
	}
}
foreach ($entry in @(
	@("image-normal", $imageNormal.data),
	@("image-clone", $imageClone.data)
)) {
	$label = [string]$entry[0]
	$manifest = $entry[1]
	if (($manifest.summary.passed -ne $true) -or
	    ([int]$manifest.summary.failed_visual_cases -ne 0) -or
	    ([int]$manifest.summary.decoder_dimension_failures -ne 0)) {
		$failures += "failed:$label"
	}
}
foreach ($entry in @(
	@("image-normal-structural", $imageNormalStructural.data.summary),
	@("image-normal-decoder", $imageNormalDecoder.data.summary),
	@("image-clone-structural", $imageCloneStructural.data.summary),
	@("image-clone-decoder", $imageCloneDecoder.data.summary)
)) {
	$label = [string]$entry[0]
	$summary = $entry[1]
	if (($summary.passed -ne $true) -or ([int]$summary.failed_compares -ne 0) -or
	    ([int]$summary.missing_references -ne 0)) {
		$failures += "failed:$label"
	}
}
foreach ($entry in @(
	@("root-aspect-normal", $rootAspectNormal.data, $rootAspectNormalChild.data.summary),
	@("root-aspect-clone", $rootAspectClone.data, $rootAspectCloneChild.data.summary)
)) {
	$label = [string]$entry[0]
	$gate = $entry[1]
	$summary = $entry[2]
	if (($gate.passed -ne $true) -or (@($gate.failures).Count -ne 0) -or
	    ($summary.passed -ne $true) -or ([int]$summary.failed_compares -ne 0) -or
	    ([int]$summary.missing_references -ne 0)) {
		$failures += "failed:$label"
	}
}

Add-DuplicateFailures "vector-normal" $vectorNormalNames ([ref]$failures)
Add-DuplicateFailures "vector-clone" $vectorCloneNames ([ref]$failures)
Add-DuplicateFailures "nontext" $nontextNames ([ref]$failures)
Add-DuplicateFailures "nontext-normal" $nontextNormalNames ([ref]$failures)
Add-DuplicateFailures "nontext-clone" $nontextCloneNames ([ref]$failures)
Add-DuplicateFailures "image-normal" $imageNormalNames ([ref]$failures)
Add-DuplicateFailures "image-clone" $imageCloneNames ([ref]$failures)
Add-DuplicateFailures "image-normal-structural" $imageNormalStructuralNames ([ref]$failures)
Add-DuplicateFailures "image-normal-decoder" $imageNormalDecoderNames ([ref]$failures)
Add-DuplicateFailures "image-clone-structural" $imageCloneStructuralNames ([ref]$failures)
Add-DuplicateFailures "image-clone-decoder" $imageCloneDecoderNames ([ref]$failures)
Add-DuplicateFailures "root-aspect-normal" $rootAspectNormalNames ([ref]$failures)
Add-DuplicateFailures "root-aspect-clone" $rootAspectCloneNames ([ref]$failures)
Add-DuplicateFailures "text" $textNames ([ref]$failures)
Add-DuplicateFailures "text-normal" $textNormalNames ([ref]$failures)
Add-DuplicateFailures "text-clone" $textCloneNames ([ref]$failures)
Add-DuplicateFailures "text-semantic" $textSemanticNames ([ref]$failures)
Add-DuplicateFailures "text-semantic-normal" $textSemanticNormalNames ([ref]$failures)
Add-DuplicateFailures "text-semantic-clone" $textSemanticCloneNames ([ref]$failures)
Add-DuplicateFailures "text-spec" $textSpecNames ([ref]$failures)
Add-DuplicateFailures "text-spec-normal" $textSpecNormalNames ([ref]$failures)
Add-DuplicateFailures "text-spec-clone" $textSpecCloneNames ([ref]$failures)
Add-DuplicateFailures "decoder-divergence" $decoderDivergenceNames ([ref]$failures)
Add-DuplicateFailures "decoder-divergence-normal" $decoderDivergenceNormalNames ([ref]$failures)
Add-DuplicateFailures "decoder-divergence-clone" $decoderDivergenceCloneNames ([ref]$failures)
Add-SetDifferenceFailures "vector-clone" $vectorNormalNames $vectorCloneNames ([ref]$failures)
Add-SetDifferenceFailures "nontext-normal" $nontextNames $nontextNormalNames ([ref]$failures)
Add-SetDifferenceFailures "nontext-clone" $nontextNames $nontextCloneNames ([ref]$failures)
Add-SetDifferenceFailures "image-clone" $imageNormalNames $imageCloneNames ([ref]$failures)
Add-SetDifferenceFailures "image-clone-structural" $imageNormalStructuralNames $imageCloneStructuralNames ([ref]$failures)
Add-SetDifferenceFailures "image-clone-decoder" $imageNormalDecoderNames $imageCloneDecoderNames ([ref]$failures)
Add-SetDifferenceFailures "root-aspect-clone" $rootAspectNormalNames $rootAspectCloneNames ([ref]$failures)
Add-SetDifferenceFailures "root-aspect-normal-wrapper" @($rootAspectNormal.data.selected_cases) $rootAspectNormalNames ([ref]$failures)
Add-SetDifferenceFailures "root-aspect-clone-wrapper" @($rootAspectClone.data.selected_cases) $rootAspectCloneNames ([ref]$failures)
Add-SetDifferenceFailures "text-normal" $textNames $textNormalNames ([ref]$failures)
Add-SetDifferenceFailures "text-clone" $textNames $textCloneNames ([ref]$failures)
Add-SetDifferenceFailures "text-semantic-normal" $textSemanticNames $textSemanticNormalNames ([ref]$failures)
Add-SetDifferenceFailures "text-semantic-clone" $textSemanticNames $textSemanticCloneNames ([ref]$failures)
Add-SetDifferenceFailures "text-spec-normal" $textSpecNames $textSpecNormalNames ([ref]$failures)
Add-SetDifferenceFailures "text-spec-clone" $textSpecNames $textSpecCloneNames ([ref]$failures)
Add-SetDifferenceFailures "decoder-divergence-normal" $decoderDivergenceNames $decoderDivergenceNormalNames ([ref]$failures)
Add-SetDifferenceFailures "decoder-divergence-clone" $decoderDivergenceNames $decoderDivergenceCloneNames ([ref]$failures)
$rootAspectNormalHashes = @{}
foreach ($entry in $rootAspectNormalChild.data.cases) {
	$rootAspectNormalHashes[[string]$entry.name] = [string]$entry.xge_sha256
}
foreach ($entry in $rootAspectCloneChild.data.cases) {
	$name = [string]$entry.name
	if ((-not $rootAspectNormalHashes.ContainsKey($name)) -or
	    ($rootAspectNormalHashes[$name] -cne [string]$entry.xge_sha256)) {
		$failures += "hash-mismatch:root-aspect-clone:$name"
	}
}
$textNormalHashes = @{}
foreach ($entry in $textNormal.data.cases) {
	$textNormalHashes[[string]$entry.name] = [string]$entry.xge_sha256
}
foreach ($entry in $textClone.data.cases) {
	$name = [string]$entry.name
	if ((-not $textNormalHashes.ContainsKey($name)) -or
	    ($textNormalHashes[$name] -cne [string]$entry.xge_sha256)) {
		$failures += "hash-mismatch:text-clone:$name"
	}
}
$textSemanticNormalHashes = @{}
foreach ($entry in $textSemanticNormal.data.cases) {
	$textSemanticNormalHashes[[string]$entry.name] = [string]$entry.xge_sha256
}
foreach ($entry in $textSemanticClone.data.cases) {
	$name = [string]$entry.name
	if ((-not $textSemanticNormalHashes.ContainsKey($name)) -or
	    ($textSemanticNormalHashes[$name] -cne [string]$entry.xge_sha256)) {
		$failures += "hash-mismatch:text-semantic-clone:$name"
	}
}
$textSpecNormalHashes = @{}
foreach ($entry in $textSpecNormal.data.cases) {
	$textSpecNormalHashes[[string]$entry.name] = [string]$entry.xge_sha256
}
foreach ($entry in $textSpecClone.data.cases) {
	$name = [string]$entry.name
	if ((-not $textSpecNormalHashes.ContainsKey($name)) -or
	    ($textSpecNormalHashes[$name] -cne [string]$entry.xge_sha256)) {
		$failures += "hash-mismatch:text-spec-clone:$name"
	}
}
$decoderDivergenceNormalHashes = @{}
foreach ($entry in $decoderDivergenceNormal.data.cases) {
	$decoderDivergenceNormalHashes[[string]$entry.name] = [string]$entry.xge_sha256
}
foreach ($entry in $decoderDivergenceClone.data.cases) {
	$name = [string]$entry.name
	if ((-not $decoderDivergenceNormalHashes.ContainsKey($name)) -or
	    ($decoderDivergenceNormalHashes[$name] -cne [string]$entry.xge_sha256)) {
		$failures += "hash-mismatch:decoder-divergence-clone:$name"
	}
}

foreach ($entry in @(
	@("image-normal", $imageNormal.data, $imageNormalStructuralNames.Count, $imageNormalDecoderNames.Count),
	@("image-clone", $imageClone.data, $imageCloneStructuralNames.Count, $imageCloneDecoderNames.Count)
)) {
	$label = [string]$entry[0]
	$manifest = $entry[1]
	$structuralCount = [int]$entry[2]
	$decoderCount = [int]$entry[3]
	if (([int]$manifest.summary.visual_gate_cases -ne $structuralCount) -or
	    ([int]$manifest.summary.decoder_report_cases -ne $decoderCount) -or
	    ([int]$manifest.summary.total_cases -ne ($structuralCount + $decoderCount))) {
		$failures += "count-mismatch:$label"
	}
}

$categoryNames = @(
	$vectorNormalNames + $nontextNames + $imageNormalNames + $rootAspectNormalNames +
	$batch156Names + $textNames + $textSemanticNames + $textSpecNames + $decoderDivergenceNames
)
Add-DuplicateFailures "coverage-categories" $categoryNames ([ref]$failures)
$coveredSet = New-NameSet $categoryNames
$catalogCases = @(Get-XgeSvgCompareCases -IncludeExperimental)
$catalogNames = @($catalogCases | ForEach-Object { [string]$_.name })
$catalogSet = New-NameSet $catalogNames
Add-DuplicateFailures "catalog" $catalogNames ([ref]$failures)
foreach ($name in $coveredSet.Keys) {
	if (-not $catalogSet.ContainsKey($name)) {
		$failures += "not-in-catalog:$name"
	}
}

$excluded = @()
foreach ($case in $catalogCases) {
	$name = [string]$case.name
	if ($coveredSet.ContainsKey($name)) {
		continue
	}
	$excluded += [ordered]@{
		name = $name
		svg = [string]$case.svg
		tags = @(Get-XgeSvgCompareCaseTags -Case $case)
	}
}
foreach ($case in $excluded) {
	if (@($case.tags) -notcontains "text") {
		$failures += "uncovered-nontext:$($case.name)"
	}
}

$outputFull = Resolve-RepoPath $OutputPath
$outputDir = Split-Path -Parent $outputFull
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
$result = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	catalog_source = (Join-Path $scriptDir "svg_compare_cases.ps1")
	manifests = [ordered]@{
		vector_normal = $vectorNormal.path
		vector_clone = $vectorClone.path
		sanitized_nontext = $nontext.path
		image_normal = $imageNormal.path
		image_clone = $imageClone.path
		convenience_api = $convenienceApi.path
		root_aspect_normal = $rootAspectNormal.path
		root_aspect_clone = $rootAspectClone.path
		batch156 = $batch156.path
		text = $text.path
		text_semantic = $textSemantic.path
		text_spec = $textSpec.path
		decoder_divergence = $decoderDivergence.path
	}
	categories = [ordered]@{
		vector_full_source = @($vectorNormalNames | Sort-Object)
		image_full_source = @($imageNormalNames | Sort-Object)
		root_aspect_full_source = @($rootAspectNormalNames | Sort-Object)
		batch156_full_source = @($batch156Names | Sort-Object)
		text_full_source = @($textNames | Sort-Object)
		sanitized_nontext_partial = @($nontextNames | Sort-Object)
		normalized_font_text_partial = @($textSemanticNames | Sort-Object)
		spec_correct_text_divergence = @($textSpecNames | Sort-Object)
		allowed_decoder_divergence = @($decoderDivergenceNames | Sort-Object)
	}
	excluded = $excluded
	failures = $failures
	summary = [ordered]@{
		catalog_cases = $catalogNames.Count
		full_source_normal_and_clone_cases = $vectorNormalNames.Count + $imageNormalNames.Count + $rootAspectNormalNames.Count + $batch156Names.Count + $textNames.Count
		sanitized_partial_normal_and_clone_cases = $nontextNames.Count
		normalized_font_partial_normal_and_clone_cases = $textSemanticNames.Count
		spec_correct_text_divergence_cases = $textSpecNames.Count
		allowed_decoder_divergence_cases = $decoderDivergenceNames.Count
		covered_case_names = $coveredSet.Count
		excluded_case_names = $excluded.Count
		convenience_api_comparisons = $convenienceExpectedComparisons
		passed = $failures.Count -eq 0
	}
}
$result | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $outputFull -Encoding UTF8

Write-Host "SVG core coverage manifest: $outputFull"
Write-Host ("  catalog={0} full-source={1} sanitized-partial={2} covered-names={3} excluded={4} failures={5} passed={6}" -f
	$result.summary.catalog_cases,
	$result.summary.full_source_normal_and_clone_cases,
	$result.summary.sanitized_partial_normal_and_clone_cases,
	$result.summary.covered_case_names,
	$result.summary.excluded_case_names,
	$failures.Count,
	$result.summary.passed)
if ($failures.Count -gt 0) {
	throw "SVG core coverage audit failed: $($failures -join ', ')"
}
