param(
	[string]$OutputPath = "res\emoji\twemoji_core\17.0",
	[string]$CachePath = "",
	[switch]$ForceDownload,
	[switch]$WriteCorpus
)

$ErrorActionPreference = "Stop"

$unicodeVersion = "17.0.0"
$unicodeEmojiUrl = "https://www.unicode.org/Public/17.0.0/emoji/emoji-test.txt"
$unicodeEmojiSha256 = "1D8A944F88D7952F7EF7C5167FEF3C67995BCAE24543949710231B03A201ACDA"
$twemojiVersion = "17.0.3"
$twemojiCommit = "b6b55fef1e8636b540a6d016a4729ca8cdf2e60b"
$twemojiUrl = "https://codeload.github.com/jdecked/twemoji/tar.gz/$twemojiCommit"
$twemojiSha256 = "705D79DE1460E5E775F362F0D0F01FBE3EF8D65BF4648C490E4649704584F747"
$skinTones = @(0x1F3FB, 0x1F3FC, 0x1F3FD, 0x1F3FE, 0x1F3FF)

function Resolve-RepositoryPath([string]$Path) {
	if ([IO.Path]::IsPathRooted($Path)) {
		return [IO.Path]::GetFullPath($Path)
	}
	return [IO.Path]::GetFullPath((Join-Path $repositoryRoot $Path))
}

function Get-VerifiedFile([string]$Url, [string]$Path, [string]$ExpectedSha256) {
	if ($ForceDownload -or -not (Test-Path -LiteralPath $Path -PathType Leaf)) {
		Write-Host "Downloading $Url"
		Invoke-WebRequest -Uri $Url -OutFile $Path
	}

	$actualSha256 = (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
	if ($actualSha256 -ne $ExpectedSha256) {
		throw "Source hash mismatch for '$Path': expected $ExpectedSha256, got $actualSha256."
	}
}

function Get-NormalizedCodepoints([string]$Sequence) {
	return @($Sequence -split "\s+" | ForEach-Object {
		([Convert]::ToUInt32($_, 16)).ToString("x")
	})
}

function Find-TwemojiAsset([string]$AssetDirectory, [string]$Sequence) {
	$codepoints = @(Get-NormalizedCodepoints $Sequence)
	$candidates = @(
		($codepoints -join "-"),
		(($codepoints | Where-Object { $_ -ne "fe0f" }) -join "-")
	) | Select-Object -Unique

	foreach ($candidate in $candidates) {
		$assetPath = Join-Path $AssetDirectory "$candidate.svg"
		if (Test-Path -LiteralPath $assetPath -PathType Leaf) {
			return Get-Item -LiteralPath $assetPath
		}
	}
	throw "Twemoji has no SVG asset for Unicode sequence '$Sequence'."
}

function Remove-ExistingOutput([string]$Path) {
	if (-not (Test-Path -LiteralPath $Path)) {
		return
	}
	$resolved = [IO.Path]::GetFullPath($Path)
	$resourceRoot = [IO.Path]::GetFullPath((Join-Path $repositoryRoot "res\emoji"))
	if (-not $resolved.StartsWith($resourceRoot + [IO.Path]::DirectorySeparatorChar,
			[StringComparison]::OrdinalIgnoreCase)) {
		throw "Refusing to replace output outside '$resourceRoot': $resolved"
	}
	Remove-Item -LiteralPath $resolved -Recurse -Force
}

$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$outputFull = Resolve-RepositoryPath $OutputPath
if ($CachePath -eq "") {
	$cacheFull = Join-Path ([IO.Path]::GetTempPath()) "xge-emoji-source-cache"
} else {
	$cacheFull = Resolve-RepositoryPath $CachePath
}
New-Item -ItemType Directory -Path $cacheFull -Force | Out-Null

$unicodeEmojiPath = Join-Path $cacheFull "emoji-test-$unicodeVersion.txt"
$twemojiArchivePath = Join-Path $cacheFull "twemoji-$twemojiCommit.tar.gz"
Get-VerifiedFile $unicodeEmojiUrl $unicodeEmojiPath $unicodeEmojiSha256
Get-VerifiedFile $twemojiUrl $twemojiArchivePath $twemojiSha256

$extractRoot = Join-Path $cacheFull "twemoji-$twemojiCommit"
if ($ForceDownload -or -not (Test-Path -LiteralPath $extractRoot -PathType Container)) {
	if (Test-Path -LiteralPath $extractRoot) {
		Remove-Item -LiteralPath $extractRoot -Recurse -Force
	}
	New-Item -ItemType Directory -Path $extractRoot | Out-Null
	tar -xf $twemojiArchivePath -C $extractRoot
}
$twemojiRoot = Get-ChildItem -LiteralPath $extractRoot -Directory | Select-Object -First 1
if ($null -eq $twemojiRoot) {
	throw "Twemoji archive did not contain a source directory."
}
$twemojiAssetDirectory = Join-Path $twemojiRoot.FullName "assets\svg"

$records = [Collections.Generic.List[object]]::new()
$order = 0
foreach ($line in Get-Content -LiteralPath $unicodeEmojiPath -Encoding UTF8) {
	if ($line -notmatch '^([0-9A-F ]+)\s*;\s*(fully-qualified|component)\s*#\s*\S+\s+E([0-9.]+)\s+(.+)$') {
		continue
	}
	$sequence = $Matches[1].Trim()
	$status = $Matches[2]
	$emojiVersion = $Matches[3]
	$name = $Matches[4].Trim()
	$codepoints = @(Get-NormalizedCodepoints $sequence)
	$hasSkinTone = $false
	foreach ($codepoint in $codepoints) {
		if ($skinTones -contains [Convert]::ToUInt32($codepoint, 16)) {
			$hasSkinTone = $true
			break
		}
	}
	if ($status -eq "fully-qualified" -and $hasSkinTone) {
		continue
	}

	$source = Find-TwemojiAsset $twemojiAssetDirectory $sequence
	$records.Add([pscustomobject]@{
		order = $order
		sequence = $sequence
		status = $status
		emoji_version = $emojiVersion
		name = $name
		asset = $source.Name
		source_path = $source.FullName
	})
	$order++
}

if ($records.Count -ne 1923) {
	throw "Expected 1923 core Emoji records for Unicode 17.0, got $($records.Count)."
}

Remove-ExistingOutput $outputFull
$svgOutput = Join-Path $outputFull "svg"
New-Item -ItemType Directory -Path $svgOutput -Force | Out-Null

$manifestLines = [Collections.Generic.List[string]]::new()
$hashLines = [Collections.Generic.List[string]]::new()
$corpusPath = Join-Path $outputFull "corpus.bin"
$corpusStream = if ($WriteCorpus) {
	[IO.File]::Open($corpusPath, [IO.FileMode]::Create, [IO.FileAccess]::Write,
		[IO.FileShare]::None)
} else {
	$null
}
$corpusHash = [Security.Cryptography.SHA256]::Create()
$corpusBytes = 0L
try {
	foreach ($record in $records) {
		$destination = Join-Path $svgOutput $record.asset
		Copy-Item -LiteralPath $record.source_path -Destination $destination
		$bytes = [IO.File]::ReadAllBytes($destination)
		if ($null -ne $corpusStream) {
			$corpusStream.Write($bytes, 0, $bytes.Length)
		}
		$corpusHash.TransformBlock($bytes, 0, $bytes.Length, $null, 0) | Out-Null
		$corpusBytes += $bytes.Length
		$hash = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash.ToLowerInvariant()
		$manifestLines.Add("$($record.sequence)|$($record.asset)|$($record.status)|$($record.emoji_version)|$($record.name)")
		$hashLines.Add("$hash  svg/$($record.asset)")
	}
} finally {
	$corpusHash.TransformFinalBlock([byte[]]::new(0), 0, 0) | Out-Null
	if ($null -ne $corpusStream) {
		$corpusStream.Dispose()
	}
}
$corpusSha256 = ([BitConverter]::ToString($corpusHash.Hash)).Replace("-", "").ToLowerInvariant()
$corpusHash.Dispose()

[IO.File]::WriteAllLines((Join-Path $outputFull "manifest.txt"), $manifestLines,
	[Text.UTF8Encoding]::new($false))
[IO.File]::WriteAllLines((Join-Path $outputFull "sha256.txt"), $hashLines,
	[Text.UTF8Encoding]::new($false))

Copy-Item -LiteralPath (Join-Path $twemojiRoot.FullName "LICENSE-GRAPHICS") -Destination $outputFull
$svgFiles = @(Get-ChildItem -LiteralPath $svgOutput -Filter "*.svg" -File)
$svgBytes = ($svgFiles | Measure-Object Length -Sum).Sum
$metadata = [ordered]@{
	format_version = 1
	dataset = "xge-twemoji-core"
	selection = [ordered]@{
		description = "Unicode fully-qualified Emoji without skin-tone sequences, plus Unicode Emoji components"
		unicode_version = $unicodeVersion
		record_count = $records.Count
		svg_count = $svgFiles.Count
		svg_bytes = [int64]$svgBytes
		corpus_bytes = $corpusBytes
	}
	sources = [ordered]@{
		unicode_emoji_test_url = $unicodeEmojiUrl
		unicode_emoji_test_sha256 = $unicodeEmojiSha256.ToLowerInvariant()
		twemoji_version = $twemojiVersion
		twemoji_commit = $twemojiCommit
		twemoji_archive_url = $twemojiUrl
		twemoji_archive_sha256 = $twemojiSha256.ToLowerInvariant()
	}
	files = [ordered]@{
		manifest_sha256 = (Get-FileHash -LiteralPath (Join-Path $outputFull "manifest.txt") -Algorithm SHA256).Hash.ToLowerInvariant()
		corpus_sha256 = $corpusSha256
		corpus_written = [bool]$WriteCorpus
	}
}
$metadata | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $outputFull "metadata.json") -Encoding UTF8

$notice = @"
# XGE Twemoji core SVG data

This generated data set contains the Unicode Emoji $unicodeVersion
fully-qualified sequences that do not contain a skin-tone modifier, plus the
nine Unicode Emoji components. It contains $($records.Count) SVG files.

Unicode sequence data:
$unicodeEmojiUrl

Graphics source:
https://github.com/jdecked/twemoji
Twemoji $twemojiVersion commit $twemojiCommit

The Twemoji graphics are licensed under CC-BY 4.0. The complete graphics
license is included as LICENSE-GRAPHICS.

Regenerate from the repository root with:
powershell -ExecutionPolicy Bypass -File tools/emoji_pack/prepare_core_svg.ps1
"@
[IO.File]::WriteAllText((Join-Path $outputFull "NOTICE.md"), $notice,
	[Text.UTF8Encoding]::new($false))

Write-Host ("Prepared {0} Emoji SVG files ({1:N0} bytes) in {2}" -f
	$svgFiles.Count, $svgBytes, $outputFull)
Write-Host ("Benchmark corpus: {0:N0} bytes, SHA-256 {1}" -f
	$metadata.selection.corpus_bytes, $metadata.files.corpus_sha256)
