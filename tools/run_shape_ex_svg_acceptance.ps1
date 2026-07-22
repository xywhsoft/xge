param(
	[string]$OutputDir = "artifacts\shape_ex_svg_acceptance_batch156",
	[switch]$SkipBuild,
	[switch]$SkipVisual,
	[switch]$SkipShapeExChildren,
	[switch]$SkipPerformance,
	[switch]$SkipResourceStress
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$outputFull = [System.IO.Path]::GetFullPath((Join-Path $root $OutputDir))

function Get-ChildPath {
	param([Parameter(Mandatory = $true)][string]$Name)

	if ([System.IO.Path]::IsPathRooted($OutputDir)) {
		return Join-Path $outputFull $Name
	}
	return Join-Path $OutputDir $Name
}

function Read-ManifestResult {
	param(
		[Parameter(Mandatory = $true)][string]$Name,
		[Parameter(Mandatory = $true)][string]$Path,
		[ValidateSet("root", "summary")]
		[string]$PassLocation = "root"
	)

	$fullPath = [System.IO.Path]::GetFullPath((Join-Path $root $Path))
	if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
		return [ordered]@{ name = $Name; manifest = $fullPath; missing = $true; passed = $false }
	}
	$data = Get-Content -LiteralPath $fullPath -Raw | ConvertFrom-Json
	$passed = if ($PassLocation -eq "summary") { $data.summary.passed -eq $true } else { $data.passed -eq $true }
	return [ordered]@{
		name = $Name
		manifest = $fullPath
		manifest_sha256 = (Get-FileHash -LiteralPath $fullPath -Algorithm SHA256).Hash
		missing = $false
		passed = $passed
	}
}

New-Item -ItemType Directory -Force -Path $outputFull | Out-Null
$failures = @()
$buildOutput = Get-ChildPath "build-gate"
if (-not $SkipBuild) {
	try {
		& (Join-Path $root "tools\run_shape_ex_svg_build_gate.ps1") -OutputDir $buildOutput
	} catch {
		$failures += "build-gate-invocation:$($_.Exception.Message)"
	}
}

$vectorNormalOutput = Get-ChildPath "visual-vector-normal"
$vectorCloneOutput = Get-ChildPath "visual-vector-clone"
$rootAspectNormalOutput = Get-ChildPath "visual-root-aspect-normal"
$rootAspectCloneOutput = Get-ChildPath "visual-root-aspect-clone"
$imageNormalOutput = Get-ChildPath "visual-image-normal"
$imageCloneOutput = Get-ChildPath "visual-image-clone"
$nontextOutput = Get-ChildPath "visual-nontext"
$textOutput = Get-ChildPath "visual-text"
$textSemanticOutput = Get-ChildPath "visual-text-semantic"
$textSpecOutput = Get-ChildPath "visual-text-spec"
$decoderDivergenceOutput = Get-ChildPath "visual-decoder-divergence"
$convenienceOutput = Get-ChildPath "visual-convenience-api"
$batch156Output = Get-ChildPath "visual-batch156"
if (-not $SkipVisual) {
	$visualGates = @(
		[ordered]@{
			name = "vector-normal"
			script = "tools\svg_compare\run_svg_visual_gate.ps1"
			params = @{ OutputDir = $vectorNormalOutput; SkipBuild = $true }
		},
		[ordered]@{
			name = "vector-clone"
			script = "tools\svg_compare\run_svg_visual_gate.ps1"
			params = @{ OutputDir = $vectorCloneOutput; SkipBuild = $true; XgeClone = $true }
		},
		[ordered]@{
			name = "root-aspect-normal"
			script = "tools\svg_compare\run_svg_root_aspect_visual_gate.ps1"
			params = @{ OutputDir = $rootAspectNormalOutput; SkipBuild = $true }
		},
		[ordered]@{
			name = "root-aspect-clone"
			script = "tools\svg_compare\run_svg_root_aspect_visual_gate.ps1"
			params = @{ OutputDir = $rootAspectCloneOutput; SkipBuild = $true; XgeClone = $true }
		},
		[ordered]@{
			name = "image-normal"
			script = "tools\svg_compare\run_svg_image_visual_gate.ps1"
			params = @{ OutputDir = $imageNormalOutput; SkipBuild = $true }
		},
		[ordered]@{
			name = "image-clone"
			script = "tools\svg_compare\run_svg_image_visual_gate.ps1"
			params = @{ OutputDir = $imageCloneOutput; SkipBuild = $true; XgeClone = $true }
		},
		[ordered]@{
			name = "nontext"
			script = "tools\svg_compare\run_svg_nontext_visual_gate.ps1"
			params = @{ OutputDir = $nontextOutput; SkipBuild = $true }
		},
		[ordered]@{
			name = "text"
			script = "tools\svg_compare\run_svg_text_visual_gate.ps1"
			params = @{ OutputDir = $textOutput; SkipBuild = $true }
		},
		[ordered]@{
			name = "text-semantic"
			script = "tools\svg_compare\run_svg_text_semantic_gate.ps1"
			params = @{ OutputDir = $textSemanticOutput; SkipBuild = $true }
		},
		[ordered]@{
			name = "text-spec"
			script = "tools\svg_compare\run_svg_text_spec_gate.ps1"
			params = @{ OutputDir = $textSpecOutput; SkipBuild = $true }
		},
		[ordered]@{
			name = "decoder-divergence"
			script = "tools\svg_compare\run_svg_decoder_divergence_gate.ps1"
			params = @{ OutputDir = $decoderDivergenceOutput; SkipBuild = $true }
		},
		[ordered]@{
			name = "convenience-api"
			script = "tools\svg_compare\run_svg_convenience_api_gate.ps1"
			params = @{ OutputDir = $convenienceOutput; SkipBuild = $true }
		},
		[ordered]@{
			name = "batch156"
			script = "tools\svg_compare\run_svg_batch156_visual_gate.ps1"
			params = @{ OutputDir = $batch156Output; SkipBuild = $true }
		}
	)
	foreach ($gate in $visualGates) {
		try {
			$gateParams = $gate.params
			& (Join-Path $root ([string]$gate.script)) @gateParams
		} catch {
			$failures += "visual-$($gate.name)-invocation:$($_.Exception.Message)"
		}
	}
}

$performanceOutput = Get-ChildPath "performance"
if (-not $SkipPerformance) {
	try {
		$performanceParams = @{
			OutputDir = $performanceOutput
			SkipBuild = $true
		}
		& (Join-Path $root "tools\svg_compare\run_svg_performance_gate.ps1") @performanceParams
	} catch {
		$failures += "performance-invocation:$($_.Exception.Message)"
	}
}

$resourceStressOutput = Get-ChildPath "resource-stress"
if (-not $SkipResourceStress) {
	try {
		$resourceStressParams = @{
			OutputDir = $resourceStressOutput
			SkipBuild = $true
		}
		& (Join-Path $root "tools\svg_compare\run_svg_mask_cache_stress.ps1") @resourceStressParams
	} catch {
		$failures += "resource-stress-invocation:$($_.Exception.Message)"
	}
}

$shapeExOutput = Get-ChildPath "shape-ex-core"
$shapeExParams = @{
	OutputDir = $shapeExOutput
	SkipBuild = $true
}
if ($SkipShapeExChildren) { $shapeExParams["SkipChildren"] = $true }
try {
	& (Join-Path $root "dev\shape_ex_compare\run_shape_ex_core_parity_gate.ps1") @shapeExParams
} catch {
	$failures += "shape-ex-core-invocation:$($_.Exception.Message)"
}
$svgCoveragePath = Get-ChildPath "svg-core\manifest.json"
try {
	$coverageParams = @{ OutputPath = $svgCoveragePath }
	if (-not $SkipVisual) {
		$coverageParams["VectorNormalManifest"] = Join-Path $vectorNormalOutput "manifest_512x512.json"
		$coverageParams["VectorCloneManifest"] = Join-Path $vectorCloneOutput "manifest_512x512.json"
		$coverageParams["NontextManifest"] = Join-Path $nontextOutput "manifest_nontext_gate_512x512.json"
		$coverageParams["TextManifest"] = Join-Path $textOutput "manifest_text_gate_512x512.json"
		$coverageParams["TextSemanticManifest"] = Join-Path $textSemanticOutput "manifest_text_semantic_gate_512x512.json"
		$coverageParams["TextSpecManifest"] = Join-Path $textSpecOutput "manifest_text_spec_gate_512x512.json"
		$coverageParams["DecoderDivergenceManifest"] = Join-Path $decoderDivergenceOutput "manifest_decoder_divergence_gate_512x512.json"
		$coverageParams["ImageNormalManifest"] = Join-Path $imageNormalOutput "manifest_image_gate_512x512.json"
		$coverageParams["ImageCloneManifest"] = Join-Path $imageCloneOutput "manifest_image_gate_512x512.json"
		$coverageParams["ConvenienceApiManifest"] = Join-Path $convenienceOutput "manifest_svg_convenience_api_gate_512x512.json"
		$coverageParams["RootAspectNormalManifest"] = Join-Path $rootAspectNormalOutput "manifest_root_aspect_gate_512x512.json"
		$coverageParams["RootAspectCloneManifest"] = Join-Path $rootAspectCloneOutput "manifest_root_aspect_gate_512x512.json"
		$coverageParams["Batch156Manifest"] = Join-Path $batch156Output "manifest_batch156_visual_gate_512x512.json"
	}
	& (Join-Path $root "tools\svg_compare\summarize_svg_core_coverage.ps1") @coverageParams
} catch {
	$failures += "svg-core-invocation:$($_.Exception.Message)"
}
$loaderPath = Get-ChildPath "thorvg-loader\manifest.json"
try {
	& (Join-Path $root "tools\svg_compare\check_thorvg_svg_loader_parity.ps1") `
		-CoreCoverageManifest $svgCoveragePath -OutputPath $loaderPath
} catch {
	$failures += "thorvg-loader-invocation:$($_.Exception.Message)"
}
$apiPath = Get-ChildPath "api-coverage\manifest.json"
try {
	& (Join-Path $root "tools\check_shape_ex_svg_api_coverage.ps1") -OutputPath $apiPath
} catch {
	$failures += "api-coverage-invocation:$($_.Exception.Message)"
}
$thorvgCoreApiPath = Get-ChildPath "thorvg-shape-ex-core-api\manifest.json"
try {
	& (Join-Path $root "dev\shape_ex_compare\check_thorvg_shape_ex_core_api_parity.ps1") -OutputPath $thorvgCoreApiPath
} catch {
	$failures += "thorvg-shape-ex-core-api-invocation:$($_.Exception.Message)"
}
$thorvgPictureApiPath = Get-ChildPath "thorvg-svg-picture-api\manifest.json"
try {
	& (Join-Path $root "dev\shape_ex_compare\check_thorvg_svg_picture_api_parity.ps1") -OutputPath $thorvgPictureApiPath
} catch {
	$failures += "thorvg-svg-picture-api-invocation:$($_.Exception.Message)"
}
$xuiBoundaryPath = Get-ChildPath "xui-boundary\manifest.json"
try {
	& (Join-Path $root "tools\check_xui_vector_boundary.ps1") -Phase integrated -OutputPath $xuiBoundaryPath
} catch {
	$failures += "xui-boundary-invocation:$($_.Exception.Message)"
}
$xuiTests = @()
foreach ($xuiTest in @(
	"test_xui\build_vector_smoke_test.bat",
	"test_xui\build_vector_xge_test.bat",
	"test_xui\build_chart_test.bat",
	"test_xui\build_proxy_xge_test.bat"
)) {
	$testPath = Join-Path $root $xuiTest
	& $testPath
	$passed = $LASTEXITCODE -eq 0
	$xuiTests += [ordered]@{ test = $xuiTest; passed = $passed; exit_code = $LASTEXITCODE }
	if (-not $passed) { $failures += "xui-test-failed:$xuiTest" }
}

$manifestResults = @()
if (-not $SkipBuild) {
	$manifestResults += Read-ManifestResult "build-gate" (Join-Path $buildOutput "manifest.json") "summary"
}
if (-not $SkipVisual) {
	$manifestResults += @(
		Read-ManifestResult "visual-vector-normal" (Join-Path $vectorNormalOutput "manifest_512x512.json") "summary"
		Read-ManifestResult "visual-vector-clone" (Join-Path $vectorCloneOutput "manifest_512x512.json") "summary"
		Read-ManifestResult "visual-root-aspect-normal" (Join-Path $rootAspectNormalOutput "manifest_root_aspect_gate_512x512.json") "root"
		Read-ManifestResult "visual-root-aspect-clone" (Join-Path $rootAspectCloneOutput "manifest_root_aspect_gate_512x512.json") "root"
		Read-ManifestResult "visual-image-normal" (Join-Path $imageNormalOutput "manifest_image_gate_512x512.json") "summary"
		Read-ManifestResult "visual-image-clone" (Join-Path $imageCloneOutput "manifest_image_gate_512x512.json") "summary"
		Read-ManifestResult "visual-nontext" (Join-Path $nontextOutput "manifest_nontext_gate_512x512.json") "summary"
		Read-ManifestResult "visual-text" (Join-Path $textOutput "manifest_text_gate_512x512.json") "summary"
		Read-ManifestResult "visual-text-semantic" (Join-Path $textSemanticOutput "manifest_text_semantic_gate_512x512.json") "summary"
		Read-ManifestResult "visual-text-spec" (Join-Path $textSpecOutput "manifest_text_spec_gate_512x512.json") "summary"
		Read-ManifestResult "visual-decoder-divergence" (Join-Path $decoderDivergenceOutput "manifest_decoder_divergence_gate_512x512.json") "summary"
		Read-ManifestResult "visual-convenience-api" (Join-Path $convenienceOutput "manifest_svg_convenience_api_gate_512x512.json") "summary"
		Read-ManifestResult "visual-batch156" (Join-Path $batch156Output "manifest_batch156_visual_gate_512x512.json") "summary"
	)
}
if (-not $SkipPerformance) {
	$manifestResults += Read-ManifestResult "performance" (Join-Path $performanceOutput "manifest_performance.json") "summary"
}
if (-not $SkipResourceStress) {
	$manifestResults += Read-ManifestResult "resource-stress" (Join-Path $resourceStressOutput "manifest_stress.json") "summary"
}
$manifestResults += @(
	Read-ManifestResult "shape-ex-core" (Join-Path $shapeExOutput "manifest_shape_ex_core_parity.json") "summary"
	Read-ManifestResult "svg-core" $svgCoveragePath "summary"
	Read-ManifestResult "thorvg-loader" $loaderPath "summary"
	Read-ManifestResult "api-coverage" $apiPath "root"
	Read-ManifestResult "thorvg-shape-ex-core-api" $thorvgCoreApiPath "summary"
	Read-ManifestResult "thorvg-svg-picture-api" $thorvgPictureApiPath "summary"
	Read-ManifestResult "xui-boundary" $xuiBoundaryPath "root"
)
foreach ($result in $manifestResults) {
	if (-not $result.passed) { $failures += "failed-manifest:$($result.name)" }
}

$sourceFiles = @(
	"xge.h", "src\xge_impl.c", "src\xge_shape_ex_coverage.c",
	"src\xge_shape_ex_stroke.c", "src\xge_shape_ex.c", "src\xge_svg.c",
	"src\xge_font.c", "src\xge_text_run.c", "src\xge_text_vector.c",
	"build_dll.bat", "build_dbg_dll.bat", "examples\xge_svg\main.c",
	"examples\xge_shape_ex_paint_order\main.c",
	"tools\run_shape_ex_svg_acceptance.ps1",
	"dev\shape_ex_compare\check_thorvg_shape_ex_core_api_parity.ps1",
	"dev\shape_ex_compare\check_thorvg_svg_picture_api_parity.ps1",
	"dev\shape_ex_compare\run_paint_order_compare.ps1",
	"dev\shape_ex_compare\thorvg_paint_order_reference.cpp",
	"tools\svg_compare\run_svg_compare.ps1",
	"tools\svg_compare\run_svg_visual_gate.ps1",
	"tools\svg_compare\run_svg_root_aspect_visual_gate.ps1",
	"tools\svg_compare\run_svg_image_visual_gate.ps1",
	"tools\svg_compare\run_svg_nontext_visual_gate.ps1",
	"tools\svg_compare\run_svg_text_visual_gate.ps1",
	"tools\svg_compare\run_svg_text_semantic_gate.ps1",
	"tools\svg_compare\run_svg_text_spec_gate.ps1",
	"tools\svg_compare\run_svg_decoder_divergence_gate.ps1",
	"tools\svg_compare\run_svg_convenience_api_gate.ps1",
	"tools\svg_compare\run_svg_batch156_visual_gate.ps1",
	"tools\svg_compare\summarize_svg_core_coverage.ps1",
	"tools\svg_compare\check_thorvg_svg_loader_parity.ps1",
	"tools\svg_compare\svg_compare_cases.ps1", "test\test_main.c",
	"test\test_text_foundation.c", "test\build_text_foundation_test.bat",
	"xui.h", "src\xui_core.c", "src\xui_internal.h", "src\xui_proxy_xge.c", "src\xui_chart.c",
	"test_xui\xui_vector_smoke_test.c", "test_xui\xui_vector_xge_test.c",
	"test_xui\xui_chart_test.c", "test_xui\xui_proxy_xge_test.c",
	"test_xui\build_vector_xge_test.bat", "tools\check_xui_vector_boundary.ps1"
)
$sourceHashes = @()
foreach ($source in $sourceFiles) {
	$path = Join-Path $root $source
	$sourceHashes += [ordered]@{
		file = $source
		sha256 = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
	}
}

$manifest = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	policy = [ordered]@{
		visual_target = "ThorVG visual consistency"
		geometry_text = "validated"
		advanced_text_features_deferred = $true
		image_decoder_differences_allowed = $true
		runtime_language = "C"
		xui_phase = "integrated"
		fresh_visual_matrix = -not $SkipVisual
	}
	manifests = $manifestResults
	xui_tests = $xuiTests
	source_hashes = $sourceHashes
	failures = $failures
	summary = [ordered]@{
		manifest_count = $manifestResults.Count
		passed_manifest_count = @($manifestResults | Where-Object { $_.passed }).Count
		failure_count = $failures.Count
		passed = $failures.Count -eq 0
	}
}
$manifestPath = Join-Path $outputFull "manifest_acceptance.json"
$manifest | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
Write-Host ("ShapeEx/SVG acceptance: manifests={0}/{1}, failures={2}, passed={3}" -f `
	$manifest.summary.passed_manifest_count, $manifest.summary.manifest_count,
	$failures.Count, $manifest.summary.passed)
Write-Host "ShapeEx/SVG acceptance manifest: $manifestPath"
if (-not $manifest.summary.passed) {
	throw "ShapeEx/SVG acceptance failed: $($failures -join '; ')"
}
