param(
	[Parameter(Mandatory = $true)]
	[string]$Path
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing

$checks = @(
	@("viewport-reverse-join", 890, 20, 96, 210, 242),
	@("include-union-left", 60, 586, 96, 210, 242),
	@("include-union-gap", 114, 586, 20, 25, 31),
	@("include-union-right", 130, 586, 96, 210, 242),
	@("include-subtract-edge", 230, 586, 239, 91, 126),
	@("include-subtract-hole", 280, 586, 20, 25, 31),
	@("nested-include-center", 446, 586, 95, 208, 151),
	@("nested-include-outside", 392, 586, 20, 25, 31),
	@("subtract-nested-include-edge", 570, 586, 240, 180, 91),
	@("subtract-nested-include-hole", 612, 586, 20, 25, 31),
	@("subtract-nested-subtract-outside", 720, 586, 169, 140, 255),
	@("subtract-nested-subtract-cut", 740, 586, 20, 25, 31),
	@("subtract-nested-subtract-restored", 778, 586, 169, 140, 255),
	@("overlap-union-first", 110, 690, 255, 126, 95),
	@("overlap-union-local-hole", 133, 690, 20, 25, 31),
	@("overlap-union-second", 160, 690, 255, 126, 95),
	@("nested-transform-center", 280, 690, 105, 218, 198),
	@("nested-transform-outside", 230, 690, 20, 25, 31),
	@("clip-rect-inside", 424, 690, 231, 117, 162),
	@("clip-rect-outside", 458, 690, 20, 25, 31),
	@("scene-shape-nested-center", 612, 690, 125, 177, 255),
	@("scene-shape-nested-outside", 570, 690, 20, 25, 31),
	@("public-shape-nested-left", 756, 690, 255, 205, 102),
	@("public-shape-nested-cut", 778, 690, 20, 25, 31),
	@("public-shape-nested-right", 800, 690, 255, 205, 102),
	@("mask-alpha-source-only", 80, 834, 20, 25, 31),
	@("mask-alpha-overlap", 116, 834, 96, 210, 242),
	@("mask-inv-alpha-source-only", 246, 834, 96, 210, 242),
	@("mask-inv-alpha-overlap", 282, 834, 20, 25, 31),
	@("mask-luma-source-only", 412, 834, 20, 25, 31),
	@("mask-luma-overlap", 448, 834, 74, 159, 183),
	@("mask-inv-luma-source-only", 578, 834, 96, 210, 242),
	@("mask-inv-luma-overlap", 614, 834, 41, 76, 89),
	@("mask-add-source-only", 744, 834, 96, 210, 242),
	@("mask-add-overlap", 780, 834, 96, 210, 242),
	@("mask-add-target-only", 816, 834, 240, 180, 91),
	@("mask-subtract-source-only", 80, 938, 96, 210, 242),
	@("mask-subtract-overlap", 116, 938, 20, 25, 31),
	@("mask-intersect-source-only", 246, 938, 20, 25, 31),
	@("mask-intersect-overlap", 282, 938, 240, 180, 91),
	@("mask-difference-source-only", 412, 938, 96, 210, 242),
	@("mask-difference-overlap", 448, 938, 20, 25, 31),
	@("mask-lighten-source-only", 578, 938, 96, 210, 242),
	@("mask-lighten-overlap", 614, 938, 96, 210, 242),
	@("mask-lighten-target-only", 650, 938, 0, 0, 0),
	@("mask-darken-source-only", 744, 938, 20, 25, 31),
	@("mask-darken-overlap", 780, 938, 96, 210, 242),
	@("effect-gaussian-edge", 58, 1108, 95, 123, 139),
	@("effect-drop-shadow", 336, 1110, 5, 7, 8),
	@("effect-fill", 400, 1110, 205, 75, 113),
	@("effect-tint", 560, 1110, 87, 167, 163),
	@("effect-tritone", 720, 1110, 165, 121, 176),
	@("nested-scene-left", 90, 1270, 50, 180, 140),
	@("nested-scene-left-child", 220, 1307, 244, 176, 68),
	@("nested-scene-right", 390, 1270, 70, 126, 222),
	@("nested-scene-third-level", 445, 1302, 250, 213, 92),
	@("nested-scene-root-overlay", 700, 1300, 226, 85, 132),
	@("scanline-aa-large-fill", 201, 1450, 251, 113, 133),
	@("scanline-aa-large-edge", 61, 1432, 118, 62, 74),
	@("scanline-aa-cached-small-fill", 440, 1450, 96, 210, 242),
	@("scanline-aa-cached-small-edge", 420, 1442, 38, 68, 80),
	@("scanline-aa-alpha-fill", 650, 1445, 143, 120, 215),
	@("scanline-aa-alpha-edge", 620, 1436, 43, 43, 65)
)

$bitmap = [System.Drawing.Bitmap]::FromFile((Resolve-Path -LiteralPath $Path))
try {
	if (($bitmap.Width -ne 900) -or ($bitmap.Height -ne 1570)) {
		throw "capture dimensions failed: actual=$($bitmap.Width)x$($bitmap.Height) expected=900x1570"
	}
	foreach ($check in $checks) {
		$color = $bitmap.GetPixel([int]$check[1], [int]$check[2])
		if (($color.R -ne [int]$check[3]) -or
			($color.G -ne [int]$check[4]) -or
			($color.B -ne [int]$check[5]) -or
			($color.A -ne 255)) {
			throw "$($check[0]) failed at $($check[1]),$($check[2]): actual=$($color.R),$($color.G),$($color.B),$($color.A) expected=$($check[3]),$($check[4]),$($check[5]),255"
		}
	}
} finally {
	$bitmap.Dispose()
}

Write-Host "ShapeEx capture passed: 10 stencil cases, 10 mask modes, 5 scene effects, nested scenes, $($checks.Count) pixel assertions"
