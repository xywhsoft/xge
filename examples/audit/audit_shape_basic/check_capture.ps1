param(
	[Parameter(Mandatory = $true)]
	[string]$Path
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing

$checks = @(
	# Section 1: Line widths - sample center of lines at x=80
	@("line-1px", 80, 20, 100, 200, 255),
	@("line-4px", 80, 86, 100, 200, 255),
	@("line-8px", 80, 174, 100, 200, 255),
	@("line-bg-gap", 80, 31, 20, 25, 31),

	# Section 2: Diagonal AA - horizontal line at 0 degrees
	@("diag-0deg", 70, 260, 255, 180, 80),
	@("diag-90deg", 750, 220, 255, 180, 80),

	# Section 3: Circles - fill and stroke of largest circle (i=5)
	@("circle-fill-large", 770, 350, 80, 220, 160),
	@("circle-stroke-large", 818, 350, 255, 100, 140),
	@("circle-fill-small", 70, 350, 80, 220, 160),

	# Section 4: Round rects - fill (purple) and stroke (yellow)
	@("roundrect-fill-r0", 80, 465, 180, 130, 255),
	@("roundrect-stroke-r0", 161, 465, 255, 220, 100),
	@("roundrect-fill-r32", 360, 565, 180, 130, 255),

	# Section 5: Arcs - pink color (arc0: center(70,700) r=35, angle=0 -> point at (105,700))
	@("arc-0", 105, 700, 255, 140, 200),
	@("arc-bg", 100, 660, 20, 25, 31),

	# Section 6: Ellipses - green fill (alpha=200 over bg -> blended)
	@("ellipse-fill-0", 80, 810, 83, 205, 163),
	@("ellipse-stroke-0", 30, 810, 255, 255, 100),

	# Section 7: Polygons
	@("polygon-triangle", 80, 930, 255, 120, 80),
	@("polygon-pentagon", 230, 920, 80, 180, 255),
	@("polygon-star", 430, 920, 200, 255, 80),

	# Section 8: Capsules (stroke at top edge y=986)
	@("capsule-fill", 80, 1000, 160, 220, 255),
	@("capsule-stroke", 210, 986, 255, 160, 100),

	# Section 9: Pixel-perfect - white shapes
	@("pxperfect-fill-normal", 50, 1080, 255, 255, 255),
	@("pxperfect-fill-snapped", 170, 1080, 255, 255, 255),

	# Section 10: Alpha overlap - non-overlapping edges
	@("alpha-red-only", 40, 1150, 137, 13, 15),
	@("alpha-green-only", 130, 1150, 10, 140, 15),

	# Background checks (avoid round-rect stroke at x<840 in y=430..600)
	@("bg-top-right", 880, 10, 20, 25, 31),
	@("bg-mid-right", 880, 650, 20, 25, 31)
)

$bitmap = [System.Drawing.Bitmap]::FromFile((Resolve-Path -LiteralPath $Path))
try {
	if (($bitmap.Width -ne 900) -or ($bitmap.Height -ne 1200)) {
		throw "capture dimensions failed: actual=$($bitmap.Width)x$($bitmap.Height) expected=900x1200"
	}
	$failCount = 0
	foreach ($check in $checks) {
		$color = $bitmap.GetPixel([int]$check[1], [int]$check[2])
		$tolerance = 2
		$dr = [Math]::Abs($color.R - [int]$check[3])
		$dg = [Math]::Abs($color.G - [int]$check[4])
		$db = [Math]::Abs($color.B - [int]$check[5])
		if (($dr -gt $tolerance) -or ($dg -gt $tolerance) -or ($db -gt $tolerance) -or ($color.A -ne 255)) {
			Write-Host "FAIL: $($check[0]) at $($check[1]),$($check[2]): actual=$($color.R),$($color.G),$($color.B),$($color.A) expected=$($check[3]),$($check[4]),$($check[5]),255 (dr=$dr,dg=$dg,db=$db)"
			$failCount++
		}
	}
	if ($failCount -gt 0) {
		throw "$failCount pixel assertion(s) failed"
	}
} finally {
	$bitmap.Dispose()
}

Write-Host "audit_shape_basic capture PASSED: $($checks.Count) pixel assertions OK"
