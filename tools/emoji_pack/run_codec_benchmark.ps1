param(
	[string]$DatasetPath = "res\emoji\twemoji_core\17.0",
	[string]$OutputPath = "artifacts\emoji_compression_benchmark",
	[int]$Iterations = 25,
	[int]$RandomPasses = 40
)

$ErrorActionPreference = "Stop"
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$datasetFull = [IO.Path]::GetFullPath((Join-Path $repositoryRoot $DatasetPath))
$outputFull = [IO.Path]::GetFullPath((Join-Path $repositoryRoot $OutputPath))
$pythonCommand = Get-Command python -ErrorAction SilentlyContinue
if ($pythonCommand -and $pythonCommand.Source -notlike "*\WindowsApps\python*.exe") {
	$python = $pythonCommand.Source
} else {
	$bundledRuntime = Get-ChildItem (Join-Path $env:USERPROFILE ".cache\codex-runtimes") `
		-Directory -ErrorAction SilentlyContinue |
		ForEach-Object { Join-Path $_.FullName "dependencies\python\python.exe" } |
		Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } |
		Select-Object -First 1
	if (-not $bundledRuntime) {
		throw "Python 3.10 or newer is required to run the codec benchmark."
	}
	$python = $bundledRuntime
}

$packageDirectory = Join-Path ([IO.Path]::GetTempPath()) "xge-emoji-benchmark-python"
if (-not (Test-Path -LiteralPath (Join-Path $packageDirectory "zstandard") -PathType Container)) {
	New-Item -ItemType Directory -Path $packageDirectory -Force | Out-Null
	& $python -m pip install --disable-pip-version-check --target $packageDirectory zstandard==0.23.0
	if ($LASTEXITCODE -ne 0) {
		throw "Unable to install the benchmark-only zstandard Python package."
	}
}

New-Item -ItemType Directory -Path $outputFull -Force | Out-Null
$gcc = (Get-Command gcc -ErrorAction Stop).Source
$xrtSource = Join-Path $PSScriptRoot "benchmark_xrt_deflate.c"
$xrtExecutable = Join-Path $outputFull "benchmark_xrt_deflate.exe"
$xrtResult = Join-Path $outputFull "xrt_deflate.json"
& $gcc -std=c11 -O2 -DNDEBUG -ffunction-sections -fdata-sections `
	$xrtSource "-Wl,--gc-sections" -s -lws2_32 -lcrypt32 -lbcrypt -lsecur32 `
	-lole32 -luuid -liphlpapi -o $xrtExecutable
if ($LASTEXITCODE -ne 0) {
	throw "Unable to build the XRT Deflate benchmark."
}
& $xrtExecutable $datasetFull $RandomPasses | Set-Content -LiteralPath $xrtResult -Encoding UTF8
if ($LASTEXITCODE -ne 0) {
	throw "XRT Deflate benchmark failed with exit code $LASTEXITCODE."
}

$oldPythonPath = $env:PYTHONPATH
try {
	$env:PYTHONPATH = if ($oldPythonPath) { "$packageDirectory;$oldPythonPath" } else { $packageDirectory }
	& $python (Join-Path $PSScriptRoot "benchmark_codecs.py") `
		--dataset $datasetFull `
		--output $outputFull `
		--iterations $Iterations `
		--random-passes $RandomPasses
	if ($LASTEXITCODE -ne 0) {
		throw "Emoji codec benchmark failed with exit code $LASTEXITCODE."
	}
} finally {
	$env:PYTHONPATH = $oldPythonPath
}

$result = Get-Content -LiteralPath (Join-Path $outputFull "results.json") -Raw | ConvertFrom-Json
Write-Host ""
Write-Host "Whole corpus"
$result.whole_corpus | Select-Object name, total_bytes,
	@{Name="ratio_pct"; Expression={[Math]::Round($_.ratio * 100.0, 2)}},
	@{Name="decode_MiB_s"; Expression={[Math]::Round($_.decompression.median_mib_s, 1)}} |
	Format-Table -AutoSize
Write-Host "Per file random access"
$result.per_file | Select-Object name, total_bytes,
	@{Name="ratio_pct"; Expression={[Math]::Round($_.ratio * 100.0, 2)}},
	@{Name="ops_s"; Expression={[Math]::Round($_.random_access.operations_per_second)}},
	@{Name="avg_us"; Expression={[Math]::Round($_.random_access.average_us, 2)}} |
	Format-Table -AutoSize
Write-Host "Results: $(Join-Path $outputFull 'results.json')"
Write-Host ""
Write-Host "XRT Deflate implementation"
$xrt = Get-Content -LiteralPath $xrtResult -Raw | ConvertFrom-Json
[pscustomobject]@{
	whole_bytes = $xrt.whole.payload_bytes
	whole_ratio_pct = [Math]::Round($xrt.whole.ratio * 100.0, 2)
	whole_decode_MiB_s = [Math]::Round($xrt.whole.decode_mib_s, 1)
	per_file_bytes = $xrt.per_file.total_bytes
	per_file_ratio_pct = [Math]::Round($xrt.per_file.ratio * 100.0, 2)
	per_file_ops_s = [Math]::Round($xrt.per_file.operations_per_second)
	per_file_avg_us = [Math]::Round($xrt.per_file.average_us, 2)
} | Format-List
Write-Host "XRT results: $xrtResult"
