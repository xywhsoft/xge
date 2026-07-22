param(
	[string]$OutputDir = "artifacts\shape_ex_svg_build_gate"
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$outputFull = [System.IO.Path]::GetFullPath((Join-Path $root $OutputDir))

function Invoke-LoggedCommand {
	param(
		[Parameter(Mandatory = $true)][string]$Name,
		[Parameter(Mandatory = $true)][string]$Command
	)

	$logPath = Join-Path $outputFull "$Name.log"
	$output = & cmd /c $Command 2>&1
	$exitCode = $LASTEXITCODE
	@($output | ForEach-Object { [string]$_ }) | Set-Content -LiteralPath $logPath -Encoding UTF8
	return [ordered]@{
		name = $Name
		command = $Command
		log = $logPath
		log_sha256 = (Get-FileHash -LiteralPath $logPath -Algorithm SHA256).Hash
		exit_code = $exitCode
		passed = $exitCode -eq 0
	}
}

New-Item -ItemType Directory -Force -Path $outputFull | Out-Null
$commands = @(
	Invoke-LoggedCommand "release-dll" "build_dll.bat"
	Invoke-LoggedCommand "svg-example" "examples\xge_svg\build.bat"
	Invoke-LoggedCommand "shape-ex-effects-example" "examples\xge_shape_ex_effects\build.bat"
	Invoke-LoggedCommand "release-test" "build_test.bat"
	Invoke-LoggedCommand "debug-test" "build_dbg_test.bat"
	Invoke-LoggedCommand "text-foundation-test" "test\build_text_foundation_test.bat"
	Invoke-LoggedCommand "c-language-boundary" "check_c_language_boundary.bat"
)
$failures = @($commands | Where-Object { -not $_.passed } | ForEach-Object { "failed-command:$($_.name)" })
$manifest = [ordered]@{
	generated_at = (Get-Date).ToString("o")
	commands = $commands
	failures = $failures
	summary = [ordered]@{
		command_count = $commands.Count
		passed_command_count = @($commands | Where-Object { $_.passed }).Count
		failure_count = $failures.Count
		passed = $failures.Count -eq 0
	}
}
$manifestPath = Join-Path $outputFull "manifest.json"
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
Write-Host ("ShapeEx/SVG build gate: commands={0}/{1}, failures={2}, passed={3}" -f `
	$manifest.summary.passed_command_count, $commands.Count, $failures.Count, $manifest.summary.passed)
Write-Host "ShapeEx/SVG build gate manifest: $manifestPath"
if (-not $manifest.summary.passed) {
	throw "ShapeEx/SVG build gate failed: $($failures -join '; ')"
}
