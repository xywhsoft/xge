param(
	[ValidateSet("isolated", "integrated")]
	[string]$Phase = "isolated",
	[string]$OutputPath = "artifacts\xui_vector_boundary\manifest.json"
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

function Resolve-RepoPath {
	param([Parameter(Mandatory = $true)][string]$Path)

	if ([System.IO.Path]::IsPathRooted($Path)) {
		return [System.IO.Path]::GetFullPath($Path)
	}
	return [System.IO.Path]::GetFullPath((Join-Path $root $Path))
}

function Get-SourceFiles {
	param([Parameter(Mandatory = $true)][string[]]$Scopes)

	$files = @()
	foreach ($scope in $Scopes) {
		$path = Resolve-RepoPath $scope
		if (Test-Path -LiteralPath $path -PathType Leaf) {
			$files += Get-Item -LiteralPath $path
		} elseif (Test-Path -LiteralPath $path -PathType Container) {
			$files += Get-ChildItem -LiteralPath $path -Recurse -File |
				Where-Object { $_.Extension -in @(".c", ".h", ".bat", ".sh") }
		}
	}
	return @($files | Sort-Object FullName -Unique)
}

function Get-Matches {
	param(
		[Parameter(Mandatory = $true)][object[]]$Files,
		[Parameter(Mandatory = $true)][string]$Pattern,
		[int]$Limit = 200
	)

	$matches = @()
	foreach ($file in $Files) {
		$lines = Get-Content -LiteralPath $file.FullName
		for ($i = 0; $i -lt $lines.Count; ++$i) {
			if (-not [regex]::IsMatch($lines[$i], $Pattern)) { continue }
			$matches += [ordered]@{
				file = $file.FullName.Substring($root.Length).TrimStart('\')
				line = $i + 1
				text = $lines[$i].Trim()
			}
			if ($matches.Count -ge $Limit) { return $matches }
		}
	}
	return $matches
}

$coreBuildFiles = Get-SourceFiles @(
	"build_dll.bat", "build_dbg_dll.bat", "build_dll.sh", "build_dbg_dll.sh",
	"build_android_ndk.bat", "build_android_ndk.sh", "platform\android\CMakeLists.txt"
)
$xuiFiles = Get-SourceFiles @("xui.h", "src") |
	Where-Object { ($_.Name -like "xui_*.c") -or ($_.Name -eq "xui.h") -or ($_.Name -eq "xui_internal.h") }
$xgeRuntimeFiles = Get-SourceFiles @("xge.c", "xge.h", "src") |
	Where-Object { ($_.Name -like "xge_*.c") -or ($_.Name -like "xge_*.h") -or ($_.Name -in @("xge.c", "xge.h")) }
$failures = @()

$xuiBuildLinks = @(Get-Matches $coreBuildFiles '(?i)xui_sources|XUI_SRC|XUI_BUILD_DLL|src[\\/]xui_[A-Za-z0-9_]+\.c')
$xgeXuiCalls = @(Get-Matches $xgeRuntimeFiles '\bxui[A-Z][A-Za-z0-9_]*\s*\(')
if ($xgeXuiCalls.Count -gt 0) {
	$failures += "xge-runtime-depends-on-xui"
}
if ($Phase -eq "isolated") {
	if ($xuiBuildLinks.Count -gt 0) {
		$failures += "isolated-build-includes-xui"
	}
} elseif ($xuiBuildLinks.Count -eq 0) {
	$failures += "integrated-build-excludes-xui"
}
$cppHeaderLinks = @(Get-Matches $xgeRuntimeFiles '(?i)#\s*include\s*[<"][^>"]+\.(?:h\+\+|hh|hpp|hxx|ixx|mpp)[>"]')
if ($cppHeaderLinks.Count -gt 0) {
	$failures += "runtime-links-cpp-header"
}

$xgeImplPath = Resolve-RepoPath "src\xge_impl.c"
$xgeImpl = Get-Content -LiteralPath $xgeImplPath -Raw
$shapeExInclude = $xgeImpl.IndexOf('#include "xge_shape_ex.c"', [System.StringComparison]::Ordinal)
$svgInclude = $xgeImpl.IndexOf('#include "xge_svg.c"', [System.StringComparison]::Ordinal)
if (($shapeExInclude -lt 0) -or ($svgInclude -lt 0) -or ($shapeExInclude -ge $svgInclude)) {
	$failures += "wrong-shapeex-svg-include-order"
}
$svgPath = Resolve-RepoPath "src\xge_svg.c"
$svgText = Get-Content -LiteralPath $svgPath -Raw
$svgShapeExCallCount = [regex]::Matches($svgText, '\bxgeShapeEx[A-Za-z0-9_]*\s*\(').Count
if ($svgShapeExCallCount -eq 0) {
	$failures += "svg-does-not-use-shapeex"
}

$xuiXgeVectorCalls = @(Get-Matches $xuiFiles '\bxge(?:ShapeEx|Svg)[A-Za-z0-9_]*\s*\(')
$xuiLegacyPathDefinitions = @(Get-Matches $xuiFiles '\b(?:xuiPathParseSvg|xuiPathBuildFillMesh|xuiPathBuildStrokeMesh|xuiPathBuildDashedStrokeMesh)\s*\(')
if ($Phase -eq "isolated") {
	if ($xuiXgeVectorCalls.Count -gt 0) {
		$failures += "isolated-xui-depends-on-new-vector-runtime"
	}
} else {
	if ($xuiXgeVectorCalls.Count -eq 0) {
		$failures += "integrated-xui-does-not-use-new-vector-runtime"
	}
	if ($xuiLegacyPathDefinitions.Count -gt 0) {
		$failures += "integrated-xui-retains-legacy-path-runtime"
	}
}

$manifest = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	phase = $Phase
	root_xui_mainline = $true
	dev_xui1_excluded = $true
	core_build = [ordered]@{
		files = @($coreBuildFiles | ForEach-Object { $_.FullName.Substring($root.Length).TrimStart('\') })
		xui_links = $xuiBuildLinks
		isolated = $xuiBuildLinks.Count -eq 0
		xui_packaged = $xuiBuildLinks.Count -gt 0
		xge_runtime_xui_calls = $xgeXuiCalls
		dependency_direction_valid = $xgeXuiCalls.Count -eq 0
	}
	runtime_language_boundary = [ordered]@{
		cpp_header_links = $cppHeaderLinks
		pure_c = $cppHeaderLinks.Count -eq 0
	}
	shapeex_svg_dependency = [ordered]@{
		shapeex_include_offset = $shapeExInclude
		svg_include_offset = $svgInclude
		include_order_valid = ($shapeExInclude -ge 0) -and ($shapeExInclude -lt $svgInclude)
		svg_shapeex_call_count = $svgShapeExCallCount
	}
	xui_vector_dependency = [ordered]@{
		new_runtime_calls = $xuiXgeVectorCalls
		legacy_path_definitions = $xuiLegacyPathDefinitions
		legacy_path_migration_pending = $xuiLegacyPathDefinitions.Count -gt 0
	}
	failures = $failures
	passed = $failures.Count -eq 0
}
$outputFull = Resolve-RepoPath $OutputPath
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $outputFull) | Out-Null
$manifest | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $outputFull -Encoding UTF8
Write-Host ("XUI vector boundary: phase={0}, xui_packaged={1}, dependency_direction_valid={2}, svg_shapeex_calls={3}, xui_new_calls={4}, legacy_path_defs={5}, failures={6}, passed={7}" -f `
	$Phase, $manifest.core_build.xui_packaged, $manifest.core_build.dependency_direction_valid, $svgShapeExCallCount, $xuiXgeVectorCalls.Count, $xuiLegacyPathDefinitions.Count, $failures.Count, $manifest.passed)
Write-Host "XUI vector boundary manifest: $outputFull"
if (-not $manifest.passed) {
	throw "XUI vector boundary audit failed: $($failures -join '; ')"
}
