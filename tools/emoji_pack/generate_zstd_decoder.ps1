param(
	[string]$Version = "1.5.7",
	[string]$PythonPath = ""
)

$ErrorActionPreference = "Stop"
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
if ($PythonPath) {
	$python = [IO.Path]::GetFullPath($PythonPath)
	if (-not (Test-Path -LiteralPath $python -PathType Leaf)) {
		throw "Python executable not found: $python"
	}
} else {
	$pythonCommand = Get-Command python3, python -CommandType Application -ErrorAction SilentlyContinue |
		Where-Object { $_.Source -notlike "*\WindowsApps\python*.exe" } |
		Select-Object -First 1
	if ($null -eq $pythonCommand) {
		throw "Python 3.10 or newer is required. Use -PythonPath to specify it."
	}
	$python = $pythonCommand.Source
}

$work = Join-Path ([IO.Path]::GetTempPath()) "xge-zstd-$Version"
$archive = Join-Path $work "zstd.tar.gz"
$source = Join-Path $work "zstd-$Version"
$url = "https://github.com/facebook/zstd/releases/download/v$Version/zstd-$Version.tar.gz"
$expectedSha256 = if ($Version -eq "1.5.7") {
	"EB33E51F49A15E023950CD7825CA74A4A2B43DB8354825AC24FC1B7EE09E6FA3"
} else {
	throw "No trusted archive hash is registered for Zstandard $Version."
}
New-Item -ItemType Directory -Path $work -Force | Out-Null
if (-not (Test-Path -LiteralPath $archive -PathType Leaf)) {
	Invoke-WebRequest -Uri $url -OutFile $archive
}
$actualSha256 = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash
if ($actualSha256 -ne $expectedSha256) {
	throw "Zstandard archive hash mismatch: $actualSha256"
}
if (-not (Test-Path -LiteralPath $source -PathType Container)) {
	tar -xzf $archive -C $work
	if ($LASTEXITCODE -ne 0) { throw "Unable to extract Zstandard source." }
}
& $python (Join-Path $source "build\single_file_libs\combine.py") `
	-r (Join-Path $source "lib") -x legacy/zstd_legacy.h `
	-o (Join-Path $root "lib\zstd\zstddeclib.c") `
	(Join-Path $root "tools\emoji_pack\xge_zstddeclib-in.c")
if ($LASTEXITCODE -ne 0) { throw "Unable to generate the Zstandard decoder." }
Write-Host "Generated lib\zstd\zstddeclib.c from official Zstandard $Version."
