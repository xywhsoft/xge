param(
	[string]$DatasetPath = "res\emoji\twemoji_core\17.0",
	[string]$DictionaryPath = "artifacts\emoji_compression_benchmark\svg-32k.dict",
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

$packageDir = Join-Path ([IO.Path]::GetTempPath()) "xge-emoji-benchmark-python"
if (-not (Test-Path -LiteralPath (Join-Path $packageDir "zstandard") -PathType Container)) {
	New-Item -ItemType Directory -Path $packageDir -Force | Out-Null
	& $python -m pip install --disable-pip-version-check --target $packageDir zstandard==0.23.0
	if ($LASTEXITCODE -ne 0) { throw "Unable to install the build-only zstandard package." }
}

$oldPythonPath = $env:PYTHONPATH
try {
	$env:PYTHONPATH = if ($oldPythonPath) { "$packageDir;$oldPythonPath" } else { $packageDir }
	$arguments = @(
		(Join-Path $PSScriptRoot "build_builtin_pack.py"),
		"--dataset", (Join-Path $root $DatasetPath),
		"--output-bin", (Join-Path $root "artifacts\xge_builtin_emoji.xep"),
		"--output-inc", (Join-Path $root "src\xge_builtin_emoji.inc")
	)
	$dictionaryFull = Join-Path $root $DictionaryPath
	if (Test-Path -LiteralPath $dictionaryFull -PathType Leaf) {
		$arguments += @("--dict", $dictionaryFull)
	}
	& $python @arguments
	if ($LASTEXITCODE -ne 0) { throw "Emoji package generation failed." }
} finally {
	$env:PYTHONPATH = $oldPythonPath
}
