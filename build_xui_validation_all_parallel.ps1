$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $Root

if ($args.Count -gt 0 -and ($args[0] -eq "/?" -or $args[0] -eq "--help" -or $args[0] -eq "help")) {
	Write-Host "[XGE] Usage: build_xui_validation_all_parallel.bat"
	Write-Host "[XGE] Builds all XUI validation EXEs in parallel."
	Write-Host "[XGE] Logs are written to build\xui_validation_logs."
	exit 0
}

if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
	Write-Host "[ERROR] gcc not found in PATH"
	exit 1
}

$OutDir = Join-Path $Root "build"
$LogDir = Join-Path $OutDir "xui_validation_logs"
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
Remove-Item -Path (Join-Path $LogDir "*.log") -Force -ErrorAction SilentlyContinue

$Flags = @("-O2", "-Wall", "-Wextra", "-Wno-unused-parameter", "-Wno-unused-function", "-Wno-cast-function-type", "-I.")
$Libs = @("-lws2_32", "-liphlpapi", "-lgdi32", "-luser32", "-lshell32", "-lopengl32", "-lole32", "-lwinmm", "-lavrt")
$Targets = @(
	@{ Name = "bridge"; Out = "build\xge_xui_bridge.exe"; Src = "examples\xui_bridge\main.c" },
	@{ Name = "incubation"; Out = "build\xge_xui_incubation.exe"; Src = "examples\xui_incubation\main.c" },
	@{ Name = "layout"; Out = "build\xge_xui_layout_validation.exe"; Src = "examples\xui_layout_validation\main.c" },
	@{ Name = "input"; Out = "build\xge_xui_input_validation.exe"; Src = "examples\xui_input_validation\main.c" },
	@{ Name = "textedit"; Out = "build\xge_xui_text_edit_validation.exe"; Src = "examples\xui_text_edit_validation\main.c" }
)

Write-Host "[XGE] Building all XUI validation EXEs in parallel..."

$Jobs = foreach ($Target in $Targets) {
	Write-Host ("[XGE] Starting {0}..." -f $Target.Name)
	Start-Job -ArgumentList $Root, $LogDir, $Target.Name, $Target.Out, $Target.Src, $Flags, $Libs -ScriptBlock {
		param($Root, $LogDir, $Name, $Out, $Src, $Flags, $Libs)
		Set-Location $Root
		$Log = Join-Path $LogDir "$Name.log"
		& gcc @Flags -o $Out xge.c $Src @Libs > $Log 2>&1
		[pscustomobject]@{
			Name = $Name
			Code = $LASTEXITCODE
			Log = $Log
		}
	}
}

Wait-Job -Job $Jobs | Out-Null
$Failed = $false

foreach ($Job in $Jobs) {
	$Result = Receive-Job -Job $Job
	Remove-Job -Job $Job
	if ($Result.Code -ne 0) {
		$Failed = $true
		Write-Host ("[XGE] Build failed: {0}" -f $Result.Name)
		Get-Content -Path $Result.Log
	} else {
		Write-Host ("[XGE] Build successful: {0}" -f $Result.Name)
	}
}

if ($Failed) {
	Write-Host "[XGE] One or more XUI validation builds failed."
	exit 1
}

Write-Host "[XGE] All XUI validation EXEs built successfully."
Write-Host "[XGE] Run:"
Write-Host "[XGE]   build\xge_xui_bridge.exe"
Write-Host "[XGE]   build\xge_xui_incubation.exe"
Write-Host "[XGE]   build\xge_xui_layout_validation.exe"
Write-Host "[XGE]   build\xge_xui_input_validation.exe"
Write-Host "[XGE]   build\xge_xui_text_edit_validation.exe"
exit 0
