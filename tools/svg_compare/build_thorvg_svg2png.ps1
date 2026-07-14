param(
	[string]$ThorvgDir = "dev\thorvg_reference",
	[string]$BuildDir = "artifacts\thorvg_reference_build",
	[string]$ToolsDir = "artifacts\thorvg_reference_tools",
	[string]$MesonExe = "",
	[switch]$BootstrapTools,
	[switch]$Clean,
	[switch]$PrintPath
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = (Resolve-Path (Join-Path $scriptDir "..\..")).Path

function Resolve-XgePath {
	param([string]$Path)

	if ([System.IO.Path]::IsPathRooted($Path)) {
		return [System.IO.Path]::GetFullPath($Path)
	}
	return [System.IO.Path]::GetFullPath((Join-Path $root $Path))
}

function Assert-ChildPath {
	param(
		[string]$Parent,
		[string]$Child
	)

	$parentFull = [System.IO.Path]::GetFullPath($Parent).TrimEnd('\', '/')
	$childFull = [System.IO.Path]::GetFullPath($Child).TrimEnd('\', '/')
	if ($childFull -eq $parentFull) {
		return
	}
	if (-not $childFull.StartsWith($parentFull + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
		throw "Refusing to operate outside parent directory: $childFull"
	}
}

function Get-CommandPathOrNull {
	param([string]$Name)

	$cmd = Get-Command $Name -ErrorAction SilentlyContinue
	if ($cmd -eq $null) {
		return $null
	}
	return $cmd.Source
}

function Ensure-LocalMesonTools {
	param([string]$ToolRoot)

	$toolRootFull = Resolve-XgePath $ToolRoot
	Assert-ChildPath -Parent $root -Child $toolRootFull
	$venvPython = Join-Path $toolRootFull "Scripts\python.exe"
	$meson = Join-Path $toolRootFull "Scripts\meson.exe"
	if (-not (Test-Path $meson)) {
		New-Item -ItemType Directory -Force -Path $toolRootFull | Out-Null
		if (-not (Test-Path $venvPython)) {
			$venvOutput = & python -m venv $toolRootFull 2>&1
			if ($LASTEXITCODE -ne 0) {
				throw "Failed to create Python venv for ThorVG tools: $toolRootFull`n$venvOutput"
			}
		}
		$pipOutput = & $venvPython -m pip install --upgrade pip meson ninja 2>&1
		if ($LASTEXITCODE -ne 0) {
			throw "Failed to install Meson/Ninja into: $toolRootFull`n$pipOutput"
		}
	}
	if (-not (Test-Path $meson)) {
		throw "Local Meson executable was not created: $meson"
	}
	$env:PATH = (Join-Path $toolRootFull "Scripts") + [System.IO.Path]::PathSeparator + $env:PATH
	return $meson
}

function Resolve-MesonExe {
	if ($MesonExe -ne "") {
		$mesonFull = Resolve-XgePath $MesonExe
		if (-not (Test-Path $mesonFull)) {
			throw "MesonExe not found: $mesonFull"
		}
		return $mesonFull
	}

	$meson = Get-CommandPathOrNull "meson"
	if ($meson -ne $null) {
		return $meson
	}

	$localMeson = Resolve-XgePath (Join-Path $ToolsDir "Scripts\meson.exe")
	if (Test-Path $localMeson) {
		$env:PATH = (Split-Path -Parent $localMeson) + [System.IO.Path]::PathSeparator + $env:PATH
		return $localMeson
	}

	if ($BootstrapTools) {
		return Ensure-LocalMesonTools -ToolRoot $ToolsDir
	}

	throw "Meson was not found. Install Meson/Ninja or rerun with -BootstrapTools."
}

$thorvgFull = Resolve-XgePath $ThorvgDir
$buildFull = Resolve-XgePath $BuildDir
Assert-ChildPath -Parent $root -Child $buildFull

if (-not (Test-Path (Join-Path $thorvgFull "meson.build"))) {
	throw "ThorVG source directory is missing meson.build: $thorvgFull"
}

if ($Clean -and (Test-Path $buildFull)) {
	Remove-Item -LiteralPath $buildFull -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $buildFull | Out-Null

$meson = Resolve-MesonExe

if (-not (Test-Path (Join-Path $buildFull "build.ninja"))) {
	$setupArgs = @(
		"setup",
		$buildFull,
		$thorvgFull,
		"--buildtype=release",
		"-Dengines=cpu",
		"-Dloaders=svg,ttf",
		"-Dtools=svg2png",
		"-Dstatic=true",
		"-Dtests=false",
		"-Dbindings=",
		"-Dsavers=",
		"-Dextra=",
		"-Dsimd=false",
		"-Dlog=false"
	)
	$setupOutput = & $meson @setupArgs 2>&1
	if ($LASTEXITCODE -ne 0) {
		throw "ThorVG Meson setup failed.`n$setupOutput"
	}
	if (-not $PrintPath) {
		$setupOutput | ForEach-Object { Write-Host $_ }
	}
}

$configureOutput = & $meson configure $buildFull "-Dloaders=svg,ttf" 2>&1
if ($LASTEXITCODE -ne 0) {
	throw "ThorVG Meson configure failed.`n$configureOutput"
}
if (-not $PrintPath) {
	$configureOutput | ForEach-Object { Write-Host $_ }
}

$compileOutput = & $meson compile -C $buildFull tvg-svg2png 2>&1
if ($LASTEXITCODE -ne 0) {
	throw "ThorVG tvg-svg2png build failed.`n$compileOutput"
}
if (-not $PrintPath) {
	$compileOutput | ForEach-Object { Write-Host $_ }
}

$renderer = Get-ChildItem -Path $buildFull -Recurse -File |
	Where-Object { ($_.Name -eq "tvg-svg2png.exe") -or ($_.Name -eq "tvg-svg2png") } |
	Select-Object -First 1
if ($renderer -eq $null) {
	throw "Could not locate built ThorVG tvg-svg2png executable in: $buildFull"
}

$runtimeDlls = @()
if ($renderer.Extension -eq ".exe") {
	$rendererDir = Split-Path -Parent $renderer.FullName
	$runtimeDlls = @(Get-ChildItem -Path $buildFull -Recurse -File |
		Where-Object { $_.Name -like "libthorvg*.dll" })
	foreach ($dll in $runtimeDlls) {
		$destDll = [System.IO.Path]::GetFullPath((Join-Path $rendererDir $dll.Name))
		$srcDll = [System.IO.Path]::GetFullPath($dll.FullName)
		if (-not $srcDll.Equals($destDll, [System.StringComparison]::OrdinalIgnoreCase)) {
			Copy-Item -LiteralPath $srcDll -Destination $destDll -Force
		}
	}
}

$result = [ordered]@{
	renderer = $renderer.FullName
	source_dir = $thorvgFull
	build_dir = $buildFull
	meson = $meson
	runtime_dlls = @($runtimeDlls | ForEach-Object { $_.FullName })
}
$resultPath = Join-Path $buildFull "xge_thorvg_svg2png.json"
$result | ConvertTo-Json -Depth 3 | Set-Content -Path $resultPath -Encoding UTF8

if ($PrintPath) {
	Write-Output $renderer.FullName
} else {
	Write-Host "ThorVG svg2png renderer: $($renderer.FullName)"
	Write-Host "ThorVG svg2png build manifest: $resultPath"
}
