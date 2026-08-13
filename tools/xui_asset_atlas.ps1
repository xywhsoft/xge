param(
	[ValidateSet("all", "pack", "codegen")]
	[string]$Mode = "all",
	[string]$Json = "res/xui_builtin_atlas.json",
	[string]$Bitmap = "res/xui_builtin_atlas.rgba.zst",
	[string]$Out = "src/xui_builtin_assets.inc",
	[string]$PythonPath = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.Drawing

function Resolve-RepoPath([string]$Path) {
	if ([System.IO.Path]::IsPathRooted($Path)) {
		return [System.IO.Path]::GetFullPath($Path)
	}
	return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Path))
}

function Convert-AssetName([string]$Name) {
	$upper = $Name.ToUpperInvariant()
	$chars = New-Object System.Text.StringBuilder
	foreach ($ch in $upper.ToCharArray()) {
		if ((($ch -ge 'A') -and ($ch -le 'Z')) -or (($ch -ge '0') -and ($ch -le '9'))) {
			[void]$chars.Append($ch)
		} else {
			[void]$chars.Append('_')
		}
	}
	return $chars.ToString()
}

function Resolve-Python([string]$RequestedPath) {
	if ($RequestedPath) {
		$resolved = [System.IO.Path]::GetFullPath($RequestedPath)
		if (!(Test-Path -LiteralPath $resolved -PathType Leaf)) {
			throw "Python executable not found: $resolved"
		}
		return $resolved
	}
	$command = Get-Command python3, python -CommandType Application -ErrorAction SilentlyContinue |
		Where-Object { $_.Source -notlike "*\WindowsApps\python*.exe" } |
		Select-Object -First 1
	if ($command) {
		return $command.Source
	}
	$bundled = Get-ChildItem (Join-Path $env:USERPROFILE ".cache\codex-runtimes") -Directory -ErrorAction SilentlyContinue |
		ForEach-Object { Join-Path $_.FullName "dependencies\python\python.exe" } |
		Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } |
		Select-Object -First 1
	if (!$bundled) {
		throw "Python 3.10 or newer is required. Use -PythonPath to specify it."
	}
	return $bundled
}

function Write-PremultipliedRgba($Bitmap, [string]$OutputPath) {
	$rect = New-Object System.Drawing.Rectangle -ArgumentList @(0, 0, $Bitmap.Width, $Bitmap.Height)
	$locked = $Bitmap.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
	try {
		$stride = [Math]::Abs($locked.Stride)
		$source = New-Object byte[] ($stride * $Bitmap.Height)
		$output = New-Object byte[] ($Bitmap.Width * $Bitmap.Height * 4)
		[System.Runtime.InteropServices.Marshal]::Copy($locked.Scan0, $source, 0, $source.Length)
		for ($y = 0; $y -lt $Bitmap.Height; $y++) {
			$srcRow = if ($locked.Stride -ge 0) { $y * $stride } else { ($Bitmap.Height - 1 - $y) * $stride }
			$dstRow = $y * $Bitmap.Width * 4
			for ($x = 0; $x -lt $Bitmap.Width; $x++) {
				$src = $srcRow + ($x * 4)
				$dst = $dstRow + ($x * 4)
				$a = [int]$source[$src + 3]
				$output[$dst + 0] = [byte][Math]::Floor((([int]$source[$src + 2] * $a) + 127) / 255.0)
				$output[$dst + 1] = [byte][Math]::Floor((([int]$source[$src + 1] * $a) + 127) / 255.0)
				$output[$dst + 2] = [byte][Math]::Floor((([int]$source[$src + 0] * $a) + 127) / 255.0)
				$output[$dst + 3] = [byte]$a
			}
		}
		[System.IO.File]::WriteAllBytes($OutputPath, $output)
	} finally {
		$Bitmap.UnlockBits($locked)
	}
}

function Save-AtlasBitmap($Config, [string]$ConfigDir, [string]$OutputPath, [string]$Python) {
	$atlas = $null
	$graphics = $null
	$rawPath = Join-Path ([System.IO.Path]::GetTempPath()) ("xui_builtin_atlas_{0}.rgba" -f [Guid]::NewGuid().ToString("N"))
	try {
		$atlas = New-Object System.Drawing.Bitmap([int]$Config.width, [int]$Config.height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
		$graphics = [System.Drawing.Graphics]::FromImage($atlas)
		$graphics.Clear([System.Drawing.Color]::Transparent)
		$graphics.CompositingMode = [System.Drawing.Drawing2D.CompositingMode]::SourceCopy
		$graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
		$graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::Half

		foreach ($asset in $Config.assets) {
			$srcPath = [System.IO.Path]::GetFullPath((Join-Path $ConfigDir ([string]$asset.source)))
			if (!(Test-Path -LiteralPath $srcPath)) {
				throw "Missing asset source: $srcPath"
			}
			$source = $null
			$prepared = $null
			try {
				$source = New-Object System.Drawing.Bitmap($srcPath)
				$srcX = 0
				$srcY = 0
				if ($asset.PSObject.Properties.Name -contains "sx") {
					$srcX = [int]$asset.sx
				}
				if ($asset.PSObject.Properties.Name -contains "sy") {
					$srcY = [int]$asset.sy
				}
				$srcW = [int]$asset.w
				$srcH = [int]$asset.h
				if (($srcX -lt 0) -or ($srcY -lt 0) -or (($srcX + $srcW) -gt $source.Width) -or (($srcY + $srcH) -gt $source.Height)) {
					throw "Asset source rect mismatch for $($asset.name): expected rect $($srcX),$($srcY),$($srcW),$($srcH), source is $($source.Width)x$($source.Height)"
				}
				if ([string]$asset.mode -eq "mask") {
					$prepared = New-Object System.Drawing.Bitmap($srcW, $srcH, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
					for ($y = 0; $y -lt $srcH; $y++) {
						for ($x = 0; $x -lt $srcW; $x++) {
							$c = $source.GetPixel($srcX + $x, $srcY + $y)
							$luma = ([int]$c.R + [int]$c.G + [int]$c.B) / 3
							if (($c.A -gt 0) -and ($luma -lt 128)) {
								$prepared.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, 255, 255, 255))
							} else {
								$prepared.SetPixel($x, $y, [System.Drawing.Color]::Transparent)
							}
						}
					}
					$graphics.DrawImage($prepared, [int]$asset.x, [int]$asset.y, [int]$asset.w, [int]$asset.h)
				} else {
					$dstRect = New-Object System.Drawing.Rectangle -ArgumentList @([int]$asset.x, [int]$asset.y, [int]$asset.w, [int]$asset.h)
					$srcRect = New-Object System.Drawing.Rectangle -ArgumentList @($srcX, $srcY, $srcW, $srcH)
					$graphics.DrawImage($source, $dstRect, $srcRect, [System.Drawing.GraphicsUnit]::Pixel)
				}
			} finally {
				if ($prepared -ne $null) { $prepared.Dispose() }
				if ($source -ne $null) { $source.Dispose() }
			}
		}
		$outDir = Split-Path -Parent $OutputPath
		if ($outDir -and !(Test-Path -LiteralPath $outDir)) {
			New-Item -ItemType Directory -Path $outDir | Out-Null
		}
		Write-PremultipliedRgba $atlas $rawPath
		$packageDir = Join-Path ([System.IO.Path]::GetTempPath()) "xge-emoji-benchmark-python"
		if (!(Test-Path -LiteralPath (Join-Path $packageDir "zstandard") -PathType Container)) {
			New-Item -ItemType Directory -Path $packageDir -Force | Out-Null
			& $Python -m pip install --disable-pip-version-check --target $packageDir zstandard==0.23.0
			if ($LASTEXITCODE -ne 0) { throw "Unable to install the build-only zstandard package." }
		}
		$oldPythonPath = $env:PYTHONPATH
		try {
			$env:PYTHONPATH = if ($oldPythonPath) { "$packageDir;$oldPythonPath" } else { $packageDir }
			& $Python (Join-Path $PSScriptRoot "xui_asset_atlas_zstd.py") --input $rawPath --output $OutputPath --level 19
			if ($LASTEXITCODE -ne 0) { throw "Unable to compress the built-in atlas." }
		} finally {
			$env:PYTHONPATH = $oldPythonPath
		}
	} finally {
		if ($graphics -ne $null) { $graphics.Dispose() }
		if ($atlas -ne $null) { $atlas.Dispose() }
		if (Test-Path -LiteralPath $rawPath) { Remove-Item -LiteralPath $rawPath -Force }
	}
}

function Write-Codegen($Config, [string]$BitmapPath, [string]$OutputPath) {
	if (!(Test-Path -LiteralPath $BitmapPath)) {
		throw "Compressed atlas bitmap does not exist: $BitmapPath"
	}
	$bytes = [System.IO.File]::ReadAllBytes($BitmapPath)
	$sb = New-Object System.Text.StringBuilder
	[void]$sb.AppendLine("/* Auto-generated by tools/xui_asset_atlas.ps1. Do not edit manually. */")
	[void]$sb.AppendLine("#ifndef XUI_BUILTIN_ASSETS_INC")
	[void]$sb.AppendLine("#define XUI_BUILTIN_ASSETS_INC")
	[void]$sb.AppendLine("")
	[void]$sb.AppendLine("#define XUI_BUILTIN_ATLAS_WIDTH $([int]$Config.width)")
	[void]$sb.AppendLine("#define XUI_BUILTIN_ATLAS_HEIGHT $([int]$Config.height)")
	[void]$sb.AppendLine("#define XUI_BUILTIN_ATLAS_STRIDE (XUI_BUILTIN_ATLAS_WIDTH * 4)")
	[void]$sb.AppendLine("#define XUI_BUILTIN_ATLAS_RGBA_SIZE (XUI_BUILTIN_ATLAS_STRIDE * XUI_BUILTIN_ATLAS_HEIGHT)")
	[void]$sb.AppendLine("#define XUI_BUILTIN_ASSET_COUNT $($Config.assets.Count)")
	[void]$sb.AppendLine("")

	$index = 0
	foreach ($asset in $Config.assets) {
		$macro = "XUI_ASSET_$(Convert-AssetName ([string]$asset.name))"
		[void]$sb.AppendLine("#define $macro $index")
		[void]$sb.AppendLine("#define ${macro}_X $([int]$asset.x)")
		[void]$sb.AppendLine("#define ${macro}_Y $([int]$asset.y)")
		[void]$sb.AppendLine("#define ${macro}_W $([int]$asset.w)")
		[void]$sb.AppendLine("#define ${macro}_H $([int]$asset.h)")
		$index++
	}

	[void]$sb.AppendLine("")
	[void]$sb.AppendLine("static const xui_builtin_asset_t g_arrXuiBuiltinAssets[XUI_BUILTIN_ASSET_COUNT] = {")
	foreach ($asset in $Config.assets) {
		[void]$sb.AppendLine("	{ ""$($asset.name)"", $([int]$asset.x), $([int]$asset.y), $([int]$asset.w), $([int]$asset.h) },")
	}
	[void]$sb.AppendLine("};")
	[void]$sb.AppendLine("")
	[void]$sb.AppendLine("static const unsigned char g_arrXuiBuiltinAtlasRgbaZstd[] = {")
	for ($i = 0; $i -lt $bytes.Length; $i += 12) {
		$count = [Math]::Min(12, $bytes.Length - $i)
		$line = New-Object System.Text.StringBuilder
		[void]$line.Append("	")
		for ($j = 0; $j -lt $count; $j++) {
			if ($j -gt 0) {
				[void]$line.Append(", ")
			}
			[void]$line.Append("0x{0:x2}" -f $bytes[$i + $j])
		}
		if (($i + $count) -lt $bytes.Length) {
			[void]$line.Append(",")
		}
		[void]$sb.AppendLine($line.ToString())
	}
	[void]$sb.AppendLine("};")
	[void]$sb.AppendLine("static const int g_iXuiBuiltinAtlasRgbaZstdSize = (int)sizeof(g_arrXuiBuiltinAtlasRgbaZstd);")
	[void]$sb.AppendLine("")
	[void]$sb.AppendLine("#endif")

	$outDir = Split-Path -Parent $OutputPath
	if ($outDir -and !(Test-Path -LiteralPath $outDir)) {
		New-Item -ItemType Directory -Path $outDir | Out-Null
	}
	[System.IO.File]::WriteAllText($OutputPath, $sb.ToString(), [System.Text.Encoding]::ASCII)
}

$jsonPath = Resolve-RepoPath $Json
$bitmapPath = Resolve-RepoPath $Bitmap
$outPath = Resolve-RepoPath $Out
$configDir = Split-Path -Parent $jsonPath
$config = Get-Content -LiteralPath $jsonPath -Raw | ConvertFrom-Json
if ([string]$config.format -ne "rgba8-premultiplied-zstd") {
	throw "Unsupported atlas format: $($config.format)"
}

if (($Mode -eq "all") -or ($Mode -eq "pack")) {
	$python = Resolve-Python $PythonPath
	Save-AtlasBitmap $config $configDir $bitmapPath $python
}
if (($Mode -eq "all") -or ($Mode -eq "codegen")) {
	Write-Codegen $config $bitmapPath $outPath
}

