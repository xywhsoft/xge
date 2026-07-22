param(
	[string]$OutputDir = "artifacts\svg_compare",
	[string]$SourceRoot = "",
	[int]$Width = 512,
	[int]$Height = 512,
	[string]$ReferenceDir = "",
	[string]$ReferenceManifest = "",
	[string]$ReferenceTag = "thorvg",
	[switch]$FailOnReferenceMissing,
	[double]$MaxDifferentPixelRatio = -1.0,
	[double]$MaxMeanChannelDiff = -1.0,
	[double]$MaxRmseChannelDiff = -1.0,
	[int]$MaxChannelDiff = -1,
	[int]$MaxPixelBoundsDelta = -1,
	[int]$AlphaBoundsThreshold = 0,
	[double]$VisualScale = 0.25,
	[int]$VisualChannelThreshold = 8,
	[double]$MaxVisualDifferentPixelRatio = -1.0,
	[double]$MaxVisualRmseChannelDiff = -1.0,
	[int]$MaxVisualChannelDiff = -1,
	[switch]$WriteDiffImages,
	[int]$DiffAmplify = 4,
	[string]$XgePreserveAspectRatio = "",
	[double]$XgeTolerance = 0.25,
	[ValidateSet("direct", "raster-file", "raster-memory", "texture-file", "texture-memory")]
	[string]$XgeRenderApi = "direct",
	[switch]$XgeClone,
	[switch]$DisableBatchRender,
	[switch]$IncludeExperimental,
	[string[]]$CaseName = @(),
	[string[]]$CaseTag = @(),
	[switch]$SkipBuild,
	[switch]$ListCases
)

$ErrorActionPreference = "Stop"

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
$referenceManifestFull = ""
$referenceManifestDir = ""
$referenceManifestData = $null
$referenceCaseByName = @{}

if (($VisualScale -le 0.0) -or ($VisualScale -gt 1.0)) {
	throw "VisualScale must be greater than 0 and no greater than 1."
}
if (($VisualChannelThreshold -lt 0) -or ($VisualChannelThreshold -gt 255)) {
	throw "VisualChannelThreshold must be between 0 and 255."
}

Add-Type -AssemblyName System.Drawing
if (-not ("XgeSvgPngMetrics" -as [type])) {
	Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

public sealed class XgeSvgPngMetrics
{
    public long DifferentPixels;
    public long DifferentPixelsGt1;
    public long DifferentPixelsGt4;
    public long DifferentPixelsAboveThreshold;
    public int MaxChannelDiff;
    public double TotalChannelDiff;
    public double TotalSquaredDiff;
    public long RawDifferentPixels;
    public int RawMaxChannelDiff;
    public double RawTotalChannelDiff;
    public double RawTotalSquaredDiff;
}

public sealed class XgeSvgPngBounds
{
    public bool NonEmpty;
    public long AlphaPixels;
    public int X;
    public int Y;
    public int Width;
    public int Height;
}

public static class XgeSvgPngScanner
{
    private static Bitmap Prepare(Bitmap source, out bool owns)
    {
        if (source.PixelFormat == PixelFormat.Format32bppArgb) {
            owns = false;
            return source;
        }
        owns = true;
        return source.Clone(new Rectangle(0, 0, source.Width, source.Height), PixelFormat.Format32bppArgb);
    }

    private static byte[] Read(Bitmap bitmap, out int stride)
    {
        Rectangle rect = new Rectangle(0, 0, bitmap.Width, bitmap.Height);
        BitmapData data = bitmap.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
        try {
            stride = data.Stride;
            int length = Math.Abs(stride) * bitmap.Height;
            byte[] pixels = new byte[length];
            Marshal.Copy(data.Scan0, pixels, 0, length);
            return pixels;
        } finally {
            bitmap.UnlockBits(data);
        }
    }

    private static int RowOffset(int y, int height, int stride)
    {
        return stride >= 0 ? y * stride : (height - 1 - y) * -stride;
    }

    public static XgeSvgPngMetrics Compare(Bitmap actualSource, Bitmap referenceSource, int threshold)
    {
        bool ownsActual;
        bool ownsReference;
        Bitmap actual = Prepare(actualSource, out ownsActual);
        Bitmap reference = Prepare(referenceSource, out ownsReference);
        try {
            int actualStride;
            int referenceStride;
            byte[] a = Read(actual, out actualStride);
            byte[] r = Read(reference, out referenceStride);
            XgeSvgPngMetrics result = new XgeSvgPngMetrics();
            for (int y = 0; y < actual.Height; y++) {
                int ai = RowOffset(y, actual.Height, actualStride);
                int ri = RowOffset(y, reference.Height, referenceStride);
                for (int x = 0; x < actual.Width; x++, ai += 4, ri += 4) {
                    int aa = a[ai + 3];
                    int ra = r[ri + 3];
                    int rawDb = Math.Abs(a[ai] - r[ri]);
                    int rawDg = Math.Abs(a[ai + 1] - r[ri + 1]);
                    int rawDr = Math.Abs(a[ai + 2] - r[ri + 2]);
                    int da = Math.Abs(aa - ra);
                    int db = Math.Abs((a[ai] * aa + 127) / 255 - (r[ri] * ra + 127) / 255);
                    int dg = Math.Abs((a[ai + 1] * aa + 127) / 255 - (r[ri + 1] * ra + 127) / 255);
                    int dr = Math.Abs((a[ai + 2] * aa + 127) / 255 - (r[ri + 2] * ra + 127) / 255);
                    int pixelDiff = dr + dg + db + da;
                    int rawPixelDiff = rawDr + rawDg + rawDb + da;
                    int pixelMax = Math.Max(Math.Max(dr, dg), Math.Max(db, da));
                    int rawPixelMax = Math.Max(Math.Max(rawDr, rawDg), Math.Max(rawDb, da));
                    if (pixelDiff != 0) result.DifferentPixels++;
                    if (pixelMax > 1) result.DifferentPixelsGt1++;
                    if (pixelMax > 4) result.DifferentPixelsGt4++;
                    if (pixelMax > threshold) result.DifferentPixelsAboveThreshold++;
                    if (pixelMax > result.MaxChannelDiff) result.MaxChannelDiff = pixelMax;
                    result.TotalChannelDiff += pixelDiff;
                    result.TotalSquaredDiff += (double)dr * dr + (double)dg * dg + (double)db * db + (double)da * da;
                    if (rawPixelDiff != 0) result.RawDifferentPixels++;
                    if (rawPixelMax > result.RawMaxChannelDiff) result.RawMaxChannelDiff = rawPixelMax;
                    result.RawTotalChannelDiff += rawPixelDiff;
                    result.RawTotalSquaredDiff += (double)rawDr * rawDr + (double)rawDg * rawDg + (double)rawDb * rawDb + (double)da * da;
                }
            }
            return result;
        } finally {
            if (ownsActual) actual.Dispose();
            if (ownsReference) reference.Dispose();
        }
    }

    public static XgeSvgPngBounds AlphaBounds(Bitmap source, int threshold)
    {
        bool owns;
        Bitmap bitmap = Prepare(source, out owns);
        try {
            int stride;
            byte[] pixels = Read(bitmap, out stride);
            int minX = bitmap.Width;
            int minY = bitmap.Height;
            int maxX = -1;
            int maxY = -1;
            long count = 0;
            for (int y = 0; y < bitmap.Height; y++) {
                int index = RowOffset(y, bitmap.Height, stride) + 3;
                for (int x = 0; x < bitmap.Width; x++, index += 4) {
                    if (pixels[index] <= threshold) continue;
                    count++;
                    if (x < minX) minX = x;
                    if (y < minY) minY = y;
                    if (x > maxX) maxX = x;
                    if (y > maxY) maxY = y;
                }
            }
            XgeSvgPngBounds result = new XgeSvgPngBounds();
            result.AlphaPixels = count;
            result.NonEmpty = maxX >= minX && maxY >= minY;
            if (result.NonEmpty) {
                result.X = minX;
                result.Y = minY;
                result.Width = maxX - minX + 1;
                result.Height = maxY - minY + 1;
            }
            return result;
        } finally {
            if (owns) bitmap.Dispose();
        }
    }
}
'@
}

function Test-XgeSourceNewerThan {
	param(
		[System.IO.FileInfo[]]$Sources,
		[string]$TargetPath
	)

	if (-not (Test-Path $TargetPath)) {
		return $true
	}
	$targetTime = (Get-Item $TargetPath).LastWriteTimeUtc
	foreach ($source in $Sources) {
		if ($source.LastWriteTimeUtc -gt $targetTime) {
			return $true
		}
	}
	return $false
}

function Compare-PngPixels {
	param(
		[string]$ActualPath,
		[string]$ReferencePath
	)

	Add-Type -AssemblyName System.Drawing
	$actual = $null
	$reference = $null
	try {
		$actual = New-Object System.Drawing.Bitmap($ActualPath)
		$reference = New-Object System.Drawing.Bitmap($ReferencePath)
		if (($actual.Width -ne $reference.Width) -or ($actual.Height -ne $reference.Height)) {
			return [ordered]@{
				width_equal = $false
				height_equal = $false
				actual_width = $actual.Width
				actual_height = $actual.Height
				reference_width = $reference.Width
				reference_height = $reference.Height
			}
		}

		$totalPixels = [int64]$actual.Width * [int64]$actual.Height
		$metrics = [XgeSvgPngScanner]::Compare($actual, $reference, 0)
		$channelCount = [double]$totalPixels * 4.0
		return [ordered]@{
			width_equal = $true
			height_equal = $true
			width = $actual.Width
			height = $actual.Height
			comparison_space = "premultiplied-rgba"
			different_pixels = $metrics.DifferentPixels
			different_pixel_ratio = if ($totalPixels -gt 0) { $metrics.DifferentPixels / [double]$totalPixels } else { 0.0 }
			different_pixels_gt_1 = $metrics.DifferentPixelsGt1
			different_pixel_ratio_gt_1 = if ($totalPixels -gt 0) { $metrics.DifferentPixelsGt1 / [double]$totalPixels } else { 0.0 }
			different_pixels_gt_4 = $metrics.DifferentPixelsGt4
			different_pixel_ratio_gt_4 = if ($totalPixels -gt 0) { $metrics.DifferentPixelsGt4 / [double]$totalPixels } else { 0.0 }
			max_channel_diff = $metrics.MaxChannelDiff
			mean_channel_diff = if ($channelCount -gt 0.0) { $metrics.TotalChannelDiff / $channelCount } else { 0.0 }
			rmse_channel_diff = if ($channelCount -gt 0.0) { [Math]::Sqrt($metrics.TotalSquaredDiff / $channelCount) } else { 0.0 }
			raw_rgba = [ordered]@{
				different_pixels = $metrics.RawDifferentPixels
				different_pixel_ratio = if ($totalPixels -gt 0) { $metrics.RawDifferentPixels / [double]$totalPixels } else { 0.0 }
				max_channel_diff = $metrics.RawMaxChannelDiff
				mean_channel_diff = if ($channelCount -gt 0.0) { $metrics.RawTotalChannelDiff / $channelCount } else { 0.0 }
				rmse_channel_diff = if ($channelCount -gt 0.0) { [Math]::Sqrt($metrics.RawTotalSquaredDiff / $channelCount) } else { 0.0 }
			}
		}
	} finally {
		if ($actual -ne $null) {
			$actual.Dispose()
		}
		if ($reference -ne $null) {
			$reference.Dispose()
		}
	}
}

function Compare-PngVisual {
	param(
		[string]$ActualPath,
		[string]$ReferencePath,
		[double]$Scale,
		[int]$ChannelThreshold
	)

	Add-Type -AssemblyName System.Drawing
	$actual = $null
	$reference = $null
	$actualVisual = $null
	$referenceVisual = $null
	$actualGraphics = $null
	$referenceGraphics = $null
	try {
		$actual = New-Object System.Drawing.Bitmap($ActualPath)
		$reference = New-Object System.Drawing.Bitmap($ReferencePath)
		if (($actual.Width -ne $reference.Width) -or ($actual.Height -ne $reference.Height)) {
			return [ordered]@{
				width_equal = $false
				height_equal = $false
				actual_width = $actual.Width
				actual_height = $actual.Height
				reference_width = $reference.Width
				reference_height = $reference.Height
			}
		}

		$visualWidth = [Math]::Max(1, [int][Math]::Round($actual.Width * $Scale))
		$visualHeight = [Math]::Max(1, [int][Math]::Round($actual.Height * $Scale))
		$actualVisual = New-Object System.Drawing.Bitmap($visualWidth, $visualHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
		$referenceVisual = New-Object System.Drawing.Bitmap($visualWidth, $visualHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
		$actualGraphics = [System.Drawing.Graphics]::FromImage($actualVisual)
		$referenceGraphics = [System.Drawing.Graphics]::FromImage($referenceVisual)
		foreach ($graphics in @($actualGraphics, $referenceGraphics)) {
			$graphics.CompositingMode = [System.Drawing.Drawing2D.CompositingMode]::SourceCopy
			$graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
			$graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
			$graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::Half
		}
		$actualGraphics.DrawImage($actual, 0, 0, $visualWidth, $visualHeight)
		$referenceGraphics.DrawImage($reference, 0, 0, $visualWidth, $visualHeight)

		$totalPixels = [int64]$visualWidth * [int64]$visualHeight
		$metrics = [XgeSvgPngScanner]::Compare($actualVisual, $referenceVisual, $ChannelThreshold)
		$channelCount = [double]$totalPixels * 4.0
		return [ordered]@{
			width_equal = $true
			height_equal = $true
			scale = $Scale
			width = $visualWidth
			height = $visualHeight
			comparison_space = "downsampled-premultiplied-rgba"
			channel_threshold = $ChannelThreshold
			different_pixels = $metrics.DifferentPixels
			different_pixel_ratio = if ($totalPixels -gt 0) { $metrics.DifferentPixels / [double]$totalPixels } else { 0.0 }
			different_pixels_above_threshold = $metrics.DifferentPixelsAboveThreshold
			different_pixel_ratio_above_threshold = if ($totalPixels -gt 0) { $metrics.DifferentPixelsAboveThreshold / [double]$totalPixels } else { 0.0 }
			max_channel_diff = $metrics.MaxChannelDiff
			mean_channel_diff = if ($channelCount -gt 0.0) { $metrics.TotalChannelDiff / $channelCount } else { 0.0 }
			rmse_channel_diff = if ($channelCount -gt 0.0) { [Math]::Sqrt($metrics.TotalSquaredDiff / $channelCount) } else { 0.0 }
		}
	} finally {
		if ($actualGraphics -ne $null) { $actualGraphics.Dispose() }
		if ($referenceGraphics -ne $null) { $referenceGraphics.Dispose() }
		if ($actualVisual -ne $null) { $actualVisual.Dispose() }
		if ($referenceVisual -ne $null) { $referenceVisual.Dispose() }
		if ($actual -ne $null) { $actual.Dispose() }
		if ($reference -ne $null) { $reference.Dispose() }
	}
}

function Write-PngDiffImage {
	param(
		[string]$ActualPath,
		[string]$ReferencePath,
		[string]$OutputPath,
		[int]$Amplify
	)

	Add-Type -AssemblyName System.Drawing
	$actual = $null
	$reference = $null
	$diff = $null
	try {
		$actual = New-Object System.Drawing.Bitmap($ActualPath)
		$reference = New-Object System.Drawing.Bitmap($ReferencePath)
		if (($actual.Width -ne $reference.Width) -or ($actual.Height -ne $reference.Height)) {
			throw "Cannot write diff image for images with different sizes: $ActualPath / $ReferencePath"
		}
		$diff = New-Object System.Drawing.Bitmap($actual.Width, $actual.Height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
		for ($y = 0; $y -lt $actual.Height; $y++) {
			for ($x = 0; $x -lt $actual.Width; $x++) {
				$a = $actual.GetPixel($x, $y)
				$r = $reference.GetPixel($x, $y)
				$aPremulR = [int](([int]$a.R * [int]$a.A + 127) / 255)
				$aPremulG = [int](([int]$a.G * [int]$a.A + 127) / 255)
				$aPremulB = [int](([int]$a.B * [int]$a.A + 127) / 255)
				$rPremulR = [int](([int]$r.R * [int]$r.A + 127) / 255)
				$rPremulG = [int](([int]$r.G * [int]$r.A + 127) / 255)
				$rPremulB = [int](([int]$r.B * [int]$r.A + 127) / 255)
				$dr = [Math]::Abs($aPremulR - $rPremulR)
				$dg = [Math]::Abs($aPremulG - $rPremulG)
				$db = [Math]::Abs($aPremulB - $rPremulB)
				$da = [Math]::Abs([int]$a.A - [int]$r.A)
				if (($dr + $dg + $db + $da) -eq 0) {
					$diff.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(0, 0, 0, 0))
				} elseif (($dr + $dg + $db) -eq 0) {
					$intensity = [Math]::Min(255, [Math]::Max(1, $da * $Amplify))
					$diff.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $intensity, 0, $intensity))
				} else {
					$red = [Math]::Min(255, [Math]::Max(1, $dr * $Amplify))
					$green = [Math]::Min(255, [Math]::Max(1, $dg * $Amplify))
					$blue = [Math]::Min(255, [Math]::Max(1, $db * $Amplify))
					$diff.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $red, $green, $blue))
				}
			}
		}
		$diff.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
	} finally {
		if ($diff -ne $null) {
			$diff.Dispose()
		}
		if ($actual -ne $null) {
			$actual.Dispose()
		}
		if ($reference -ne $null) {
			$reference.Dispose()
		}
	}
}

function Convert-XgeSvgBoundsRect {
	param(
		[string]$Line
	)

	$number = "([-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?)"
	if ($Line -notmatch ("^\s*(local|draw)\s+{0}\s+{0}\s+{0}\s+{0}" -f $number)) {
		return $null
	}
	$culture = [System.Globalization.CultureInfo]::InvariantCulture
	return [ordered]@{
		x = [double]::Parse($matches[2], $culture)
		y = [double]::Parse($matches[3], $culture)
		w = [double]::Parse($matches[4], $culture)
		h = [double]::Parse($matches[5], $culture)
	}
}

function Get-PngAlphaBounds {
	param(
		[string]$Path,
		[int]$Threshold
	)

	Add-Type -AssemblyName System.Drawing
	$bitmap = $null
	try {
		$bitmap = New-Object System.Drawing.Bitmap($Path)
		$bounds = [XgeSvgPngScanner]::AlphaBounds($bitmap, $Threshold)
		if (-not $bounds.NonEmpty) {
			return [ordered]@{
				non_empty = $false
				alpha_pixel_count = 0
				x = 0
				y = 0
				w = 0
				h = 0
			}
		}
		return [ordered]@{
			non_empty = $true
			alpha_pixel_count = $bounds.AlphaPixels
			x = $bounds.X
			y = $bounds.Y
			w = $bounds.Width
			h = $bounds.Height
		}
	} finally {
		if ($bitmap -ne $null) {
			$bitmap.Dispose()
		}
	}
}

function Compare-PngAlphaBounds {
	param(
		[object]$Actual,
		[object]$Reference
	)

	$actualNonEmpty = [bool]$Actual.non_empty
	$referenceNonEmpty = [bool]$Reference.non_empty
	if (-not $actualNonEmpty -and -not $referenceNonEmpty) {
		return [ordered]@{
			empty_equal = $true
			actual_non_empty = $false
			reference_non_empty = $false
			dx = 0
			dy = 0
			dw = 0
			dh = 0
			max_abs_delta = 0
		}
	}
	if ($actualNonEmpty -ne $referenceNonEmpty) {
		return [ordered]@{
			empty_equal = $false
			actual_non_empty = $actualNonEmpty
			reference_non_empty = $referenceNonEmpty
			dx = $null
			dy = $null
			dw = $null
			dh = $null
			max_abs_delta = $null
		}
	}
	$dx = [int]$Actual.x - [int]$Reference.x
	$dy = [int]$Actual.y - [int]$Reference.y
	$dw = [int]$Actual.w - [int]$Reference.w
	$dh = [int]$Actual.h - [int]$Reference.h
	return [ordered]@{
		empty_equal = $true
		actual_non_empty = $actualNonEmpty
		reference_non_empty = $referenceNonEmpty
		dx = $dx
		dy = $dy
		dw = $dw
		dh = $dh
		max_abs_delta = [Math]::Max([Math]::Max([Math]::Abs($dx), [Math]::Abs($dy)), [Math]::Max([Math]::Abs($dw), [Math]::Abs($dh)))
	}
}

function Read-XgeSvgBounds {
	param(
		[string]$Exe,
		[string]$SvgPath,
		[int]$Width,
		[int]$Height,
		[string]$Aspect
	)

	$args = @("--bounds", $SvgPath, "--width", $Width, "--height", $Height)
	if ($Aspect -ne "") {
		$args += @("--aspect", $Aspect)
	}
	$output = & $Exe @args 2>&1
	if ($LASTEXITCODE -ne 0) {
		throw "XGE SVG bounds failed for ${SvgPath}: $output"
	}
	$local = $null
	$draw = $null
	foreach ($lineObject in $output) {
		$line = [string]$lineObject
		if ($line -match "^\s*local\s+") {
			$local = Convert-XgeSvgBoundsRect -Line $line
		} elseif ($line -match "^\s*draw\s+") {
			$draw = Convert-XgeSvgBoundsRect -Line $line
		}
	}
	if (($local -eq $null) -or ($draw -eq $null)) {
		throw "XGE SVG bounds output could not be parsed for ${SvgPath}: $output"
	}
	return [ordered]@{
		local = $local
		draw = $draw
	}
}

if ($ReferenceManifest -ne "") {
	$referenceManifestFull = if ([System.IO.Path]::IsPathRooted($ReferenceManifest)) { $ReferenceManifest } else { Join-Path $root $ReferenceManifest }
	if (-not (Test-Path $referenceManifestFull)) {
		throw "ReferenceManifest not found: $referenceManifestFull"
	}
	$referenceManifestFull = (Resolve-Path $referenceManifestFull).Path
	$referenceManifestDir = Split-Path -Parent $referenceManifestFull
	$referenceManifestData = Get-Content -LiteralPath $referenceManifestFull -Raw | ConvertFrom-Json

	$referenceTagProperty = $referenceManifestData.PSObject.Properties["reference_tag"]
	if (($referenceTagProperty -ne $null) -and ($referenceTagProperty.Value -ne $null) -and (-not $PSBoundParameters.ContainsKey("ReferenceTag"))) {
		$ReferenceTag = [string]$referenceTagProperty.Value
	}

	$widthProperty = $referenceManifestData.PSObject.Properties["width"]
	if (($widthProperty -ne $null) -and ($widthProperty.Value -ne $null)) {
		$manifestWidth = [int]$widthProperty.Value
		if (-not $PSBoundParameters.ContainsKey("Width")) {
			$Width = $manifestWidth
		} elseif ($manifestWidth -ne $Width) {
			throw "ReferenceManifest width ($manifestWidth) does not match requested Width ($Width)."
		}
	}

	$heightProperty = $referenceManifestData.PSObject.Properties["height"]
	if (($heightProperty -ne $null) -and ($heightProperty.Value -ne $null)) {
		$manifestHeight = [int]$heightProperty.Value
		if (-not $PSBoundParameters.ContainsKey("Height")) {
			$Height = $manifestHeight
		} elseif ($manifestHeight -ne $Height) {
			throw "ReferenceManifest height ($manifestHeight) does not match requested Height ($Height)."
		}
	}

	if ($referenceManifestData.PSObject.Properties["cases"] -eq $null) {
		throw "ReferenceManifest does not contain a cases array: $referenceManifestFull"
	}

	foreach ($refCase in @($referenceManifestData.cases)) {
		if ($refCase -eq $null) {
			continue
		}
		$nameProperty = $refCase.PSObject.Properties["name"]
		$pngProperty = $refCase.PSObject.Properties["reference_png"]
		if (($nameProperty -eq $null) -or ($nameProperty.Value -eq $null) -or ($pngProperty -eq $null) -or ($pngProperty.Value -eq $null)) {
			continue
		}
		$refPng = [string]$pngProperty.Value
		if (-not [System.IO.Path]::IsPathRooted($refPng)) {
			$refPng = Join-Path $referenceManifestDir $refPng
		}
		$referenceCaseByName[[string]$nameProperty.Value] = [ordered]@{
			reference_png = $refPng
			manifest_case = $refCase
		}
	}
}

if (($Width -le 0) -or ($Height -le 0)) {
	throw "Width and Height must be positive."
}
if (($AlphaBoundsThreshold -lt 0) -or ($AlphaBoundsThreshold -gt 255)) {
	throw "AlphaBoundsThreshold must be in the 0..255 range."
}
if ($DiffAmplify -le 0) {
	throw "DiffAmplify must be positive."
}
if ([double]::IsNaN($XgeTolerance) -or [double]::IsInfinity($XgeTolerance) -or ($XgeTolerance -le 0.0)) {
	throw "XgeTolerance must be finite and positive."
}
if ($XgeRenderApi -ne "direct") {
	if ($XgeClone) {
		throw "XgeClone is only available with the direct render API."
	}
	if ($XgePreserveAspectRatio -ne "") {
		throw "XgePreserveAspectRatio is only available with the direct render API."
	}
}
$xgeToleranceArg = $XgeTolerance.ToString("R", [Globalization.CultureInfo]::InvariantCulture)
if ($PSBoundParameters.ContainsKey("CaseTag")) {
	$effectiveCaseTags = @($CaseTag | Where-Object { ($_ -ne $null) -and ($_ -ne "") })
	if ($effectiveCaseTags.Count -eq 0) {
		throw "SVG compare case tag filter is empty."
	}
}

. (Join-Path $scriptDir "svg_compare_cases.ps1")

$cases = Select-XgeSvgCompareCases -Cases (Get-XgeSvgCompareCases -IncludeExperimental:$IncludeExperimental) -CaseName $CaseName -CaseTag $CaseTag
if ($ListCases) {
	Write-XgeSvgCompareCaseList -Cases $cases
	return
}

Push-Location $root
	try {
		$exe = Join-Path $root "build\xge_svg.exe"
		if (-not $SkipBuild) {
			$dll = Join-Path $root "build\xge.dll"
			$lib = Join-Path $root "build\xge.lib"
			$engineSources = @(
				Get-ChildItem -Path $root -File | Where-Object { $_.Extension -in ".c", ".h", ".inl" }
				Get-ChildItem -Path (Join-Path $root "src") -Recurse -File | Where-Object { $_.Extension -in ".c", ".h", ".inl" }
			)
			$dllRebuilt = $false
			if ((Test-XgeSourceNewerThan -Sources $engineSources -TargetPath $dll) -or (-not (Test-Path $lib))) {
				& (Join-Path $root "build_dll.bat")
				if ($LASTEXITCODE -ne 0) {
					throw "Failed to build xge.dll."
				}
				$dllRebuilt = $true
			}
			$exampleSources = @(
				Get-Item (Join-Path $root "examples\xge_svg\main.c")
				Get-Item $lib
			)
			if ($dllRebuilt -or (Test-XgeSourceNewerThan -Sources $exampleSources -TargetPath $exe)) {
				& (Join-Path $root "examples\xge_svg\build.bat")
				if ($LASTEXITCODE -ne 0) {
					throw "Failed to build xge_svg.exe."
				}
			}
		} elseif (-not (Test-Path $exe)) {
			throw "xge_svg.exe is missing while SkipBuild is set."
		}

	$outDirFull = if ([System.IO.Path]::IsPathRooted($OutputDir)) { $OutputDir } else { Join-Path $root $OutputDir }
	New-Item -ItemType Directory -Force -Path $outDirFull | Out-Null
	$refDirFull = $null
	if ($ReferenceDir -ne "") {
		$refDirFull = if ([System.IO.Path]::IsPathRooted($ReferenceDir)) { $ReferenceDir } else { Join-Path $root $ReferenceDir }
	} elseif ($ReferenceManifest -ne "") {
		$refDirFull = $referenceManifestDir
	}
	if (($ReferenceDir -ne "") -or ($ReferenceManifest -ne "")) {
		if ($ReferenceTag -eq "") {
			throw "ReferenceTag must not be empty when a reference source is set."
		}
	}

	$results = @()
	$comparedCount = 0
	$missingReferenceCount = 0
	$failedCompareCount = 0
	$useBatchRender = (-not $DisableBatchRender) -and ($cases.Count -gt 1)
	if ($useBatchRender) {
		$batchList = Join-Path $outDirFull ("render_jobs_{0}x{1}.tsv" -f $Width, $Height)
		$batchLines = @()
		foreach ($case in $cases) {
			$svgPath = Join-Path $sourceRootFull $case.svg
			$outPng = Join-Path $outDirFull ("{0}_xge_{1}x{2}.png" -f $case.name, $Width, $Height)
			$caseAspect = $XgePreserveAspectRatio
			if ($case.Contains("xge_preserve_aspect_ratio")) {
				$caseAspect = [string]$case.xge_preserve_aspect_ratio
				if ($caseAspect -ceq "document") { $caseAspect = "" }
			}
			if (($svgPath -match "`t") -or ($outPng -match "`t") -or ($caseAspect -match "`t")) {
				throw "Batch render paths and aspect values must not contain tabs."
			}
			$batchLines += "${svgPath}`t${outPng}`t${caseAspect}"
		}
		[System.IO.File]::WriteAllLines(
			$batchList,
			[string[]]$batchLines,
			(New-Object System.Text.UTF8Encoding($false))
		)
		$batchArgs = @("--render-list", $batchList, "--width", $Width, "--height", $Height, "--render-api", $XgeRenderApi)
		if ($XgeRenderApi -eq "direct") { $batchArgs += @("--tolerance", $xgeToleranceArg) }
		if ($XgeClone) { $batchArgs += "--clone" }
		& $exe @batchArgs
		if ($LASTEXITCODE -ne 0) {
			throw "XGE SVG batch render failed."
		}
	}

	foreach ($case in $cases) {
		$svgPath = Join-Path $sourceRootFull $case.svg
		if (-not (Test-Path -LiteralPath $svgPath -PathType Leaf)) {
			throw "SVG case source not found: $svgPath"
		}
		$outPng = Join-Path $outDirFull ("{0}_xge_{1}x{2}.png" -f $case.name, $Width, $Height)
		$caseAspect = $XgePreserveAspectRatio
		if ($case.Contains("xge_preserve_aspect_ratio")) {
			$caseAspect = [string]$case.xge_preserve_aspect_ratio
			if ($caseAspect -ceq "document") { $caseAspect = "" }
		}
		$bounds = Read-XgeSvgBounds -Exe $exe -SvgPath $svgPath -Width $Width -Height $Height -Aspect $caseAspect

		if (-not $useBatchRender) {
			$renderArgs = @("--render", $svgPath, "--width", $Width, "--height", $Height, "--capture", $outPng, "--render-api", $XgeRenderApi)
			if ($XgeRenderApi -eq "direct") {
				$renderArgs += @("--tolerance", $xgeToleranceArg)
			}
			if ($caseAspect -ne "") {
				$renderArgs += @("--aspect", $caseAspect)
			}
			if ($XgeClone) {
				$renderArgs += "--clone"
			}
			& $exe @renderArgs
			if ($LASTEXITCODE -ne 0) {
				throw "XGE SVG render failed for $($case.name)."
			}
		} elseif (-not (Test-Path -LiteralPath $outPng -PathType Leaf)) {
			throw "XGE SVG batch render did not create output for $($case.name): $outPng"
		}
		$xgePixelBounds = Get-PngAlphaBounds -Path $outPng -Threshold $AlphaBoundsThreshold

		$entry = [ordered]@{
			name = $case.name
			svg = $case.svg
			width = $Width
			height = $Height
			tags = @(Get-XgeSvgCompareCaseTags -Case $case)
			xge_preserve_aspect_ratio = if ($caseAspect -ne "") { $caseAspect } else { $null }
			xge_preserve_aspect_ratio_source = if ($case.Contains("xge_preserve_aspect_ratio") -and ([string]$case.xge_preserve_aspect_ratio -ceq "document")) { "document" } else { "override" }
			xge_png = (Resolve-Path $outPng).Path
			xge_sha256 = (Get-FileHash -Algorithm SHA256 $outPng).Hash.ToLowerInvariant()
			xge_bounds = $bounds
			xge_pixel_bounds = $xgePixelBounds
		}
		if ($case.Contains("experimental")) {
			$entry["experimental"] = $case.experimental
		}

		if (($ReferenceDir -ne "") -or ($ReferenceManifest -ne "")) {
			$refPng = $null
			if ($referenceCaseByName.ContainsKey([string]$case.name)) {
				$refPng = [string]$referenceCaseByName[[string]$case.name].reference_png
			} elseif ($refDirFull -ne $null) {
				$refPng = Join-Path $refDirFull ("{0}_{1}_{2}x{3}.png" -f $case.name, $ReferenceTag, $Width, $Height)
			}
			if (Test-Path $refPng) {
				$comparedCount++
				$referencePixelBounds = Get-PngAlphaBounds -Path $refPng -Threshold $AlphaBoundsThreshold
				$pixelBoundsDiff = Compare-PngAlphaBounds -Actual $xgePixelBounds -Reference $referencePixelBounds
				$entry["reference_png"] = (Resolve-Path $refPng).Path
				$entry["reference_sha256"] = (Get-FileHash -Algorithm SHA256 $refPng).Hash.ToLowerInvariant()
				$entry["reference_pixel_bounds"] = $referencePixelBounds
				$entry["pixel_bounds_diff"] = $pixelBoundsDiff
				$entry["hash_equal"] = ($entry["xge_sha256"] -eq $entry["reference_sha256"])
				$pixelDiff = Compare-PngPixels -ActualPath $outPng -ReferencePath $refPng
				$visualDiff = Compare-PngVisual -ActualPath $outPng -ReferencePath $refPng -Scale $VisualScale -ChannelThreshold $VisualChannelThreshold
				if ($WriteDiffImages) {
					$diffPng = Join-Path $outDirFull ("{0}_diff_{1}_{2}x{3}.png" -f $case.name, $ReferenceTag, $Width, $Height)
					Write-PngDiffImage -ActualPath $outPng -ReferencePath $refPng -OutputPath $diffPng -Amplify $DiffAmplify
					$entry["diff_png"] = (Resolve-Path $diffPng).Path
					$entry["diff_amplify"] = $DiffAmplify
				}
				$thresholdFailures = @()
				if (-not $pixelDiff["width_equal"] -or -not $pixelDiff["height_equal"]) {
					$thresholdFailures += "size"
				} else {
					if (($MaxDifferentPixelRatio -ge 0.0) -and ([double]$pixelDiff["different_pixel_ratio"] -gt $MaxDifferentPixelRatio)) {
						$thresholdFailures += "different_pixel_ratio"
					}
					if (($MaxMeanChannelDiff -ge 0.0) -and ([double]$pixelDiff["mean_channel_diff"] -gt $MaxMeanChannelDiff)) {
						$thresholdFailures += "mean_channel_diff"
					}
					if (($MaxRmseChannelDiff -ge 0.0) -and ([double]$pixelDiff["rmse_channel_diff"] -gt $MaxRmseChannelDiff)) {
						$thresholdFailures += "rmse_channel_diff"
					}
					if (($MaxChannelDiff -ge 0) -and ([int]$pixelDiff["max_channel_diff"] -gt $MaxChannelDiff)) {
						$thresholdFailures += "max_channel_diff"
					}
					if (($MaxPixelBoundsDelta -ge 0) -and ((-not [bool]$pixelBoundsDiff["empty_equal"]) -or ([int]$pixelBoundsDiff["max_abs_delta"] -gt $MaxPixelBoundsDelta))) {
						$thresholdFailures += "pixel_bounds"
					}
					if (($MaxVisualDifferentPixelRatio -ge 0.0) -and ([double]$visualDiff["different_pixel_ratio_above_threshold"] -gt $MaxVisualDifferentPixelRatio)) {
						$thresholdFailures += "visual_different_pixel_ratio"
					}
					if (($MaxVisualRmseChannelDiff -ge 0.0) -and ([double]$visualDiff["rmse_channel_diff"] -gt $MaxVisualRmseChannelDiff)) {
						$thresholdFailures += "visual_rmse_channel_diff"
					}
					if (($MaxVisualChannelDiff -ge 0) -and ([int]$visualDiff["max_channel_diff"] -gt $MaxVisualChannelDiff)) {
						$thresholdFailures += "visual_max_channel_diff"
					}
				}
				$entry["pixel_diff"] = $pixelDiff
				$entry["visual_diff"] = $visualDiff
				$entry["within_threshold"] = ($thresholdFailures.Count -eq 0)
				if ($thresholdFailures.Count -gt 0) {
					$entry["threshold_failures"] = $thresholdFailures
					$failedCompareCount++
				}
			} else {
				$missingReferenceCount++
				$entry["reference_missing"] = if ($refPng -ne $null) { $refPng } else { $case.name }
				if ($FailOnReferenceMissing) {
					$entry["within_threshold"] = $false
					$entry["threshold_failures"] = @("reference_missing")
					$failedCompareCount++
				}
			}
		}

		$results += $entry
	}

	$manifest = [ordered]@{
		generator = "xge svg compare"
		source_root = $sourceRootFull
		width = $Width
		height = $Height
		xge = [ordered]@{
			mode = if ($XgeRenderApi -ne "direct") { $XgeRenderApi } elseif ($XgeClone) { "clone" } else { "normal" }
			render_api = $XgeRenderApi
			clone = [bool]$XgeClone
		}
		case_filters = [ordered]@{
			include_experimental = [bool]$IncludeExperimental
			case_name = @($CaseName)
			case_tag = @($CaseTag)
			xge_preserve_aspect_ratio = if ($XgePreserveAspectRatio -ne "") { $XgePreserveAspectRatio } else { $null }
			xge_tolerance = if ($XgeRenderApi -eq "direct") { $XgeTolerance } else { $null }
			xge_render_api = $XgeRenderApi
			xge_batch_render = $useBatchRender
		}
		reference_dir = if ($refDirFull -ne $null) { $refDirFull } else { $ReferenceDir }
		reference_manifest = if ($ReferenceManifest -ne "") { $referenceManifestFull } else { $null }
		reference_tag = if (($ReferenceDir -ne "") -or ($ReferenceManifest -ne "")) { $ReferenceTag } else { $null }
		thresholds = [ordered]@{
			fail_on_reference_missing = [bool]$FailOnReferenceMissing
			max_different_pixel_ratio = $MaxDifferentPixelRatio
			max_mean_channel_diff = $MaxMeanChannelDiff
			max_rmse_channel_diff = $MaxRmseChannelDiff
			max_channel_diff = $MaxChannelDiff
			max_pixel_bounds_delta = $MaxPixelBoundsDelta
			alpha_bounds_threshold = $AlphaBoundsThreshold
			visual_scale = $VisualScale
			visual_channel_threshold = $VisualChannelThreshold
			max_visual_different_pixel_ratio = $MaxVisualDifferentPixelRatio
			max_visual_rmse_channel_diff = $MaxVisualRmseChannelDiff
			max_visual_channel_diff = $MaxVisualChannelDiff
			write_diff_images = [bool]$WriteDiffImages
			diff_amplify = if ($WriteDiffImages) { $DiffAmplify } else { $null }
		}
		summary = [ordered]@{
			total_cases = $results.Count
			compared_cases = $comparedCount
			missing_references = $missingReferenceCount
			failed_compares = $failedCompareCount
			passed = ($failedCompareCount -eq 0)
		}
		cases = $results
	}
	$manifestPath = Join-Path $outDirFull ("manifest_{0}x{1}.json" -f $Width, $Height)
	$manifest | ConvertTo-Json -Depth 5 | Set-Content -Path $manifestPath -Encoding UTF8
	Write-Host "SVG compare manifest saved: $manifestPath"
	if ($failedCompareCount -gt 0) {
		throw "SVG compare failed: $failedCompareCount case(s) exceeded thresholds or missing references."
	}
} finally {
	Pop-Location
}
