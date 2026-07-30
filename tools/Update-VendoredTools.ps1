#Requires -Version 7.0
<#
.SYNOPSIS
    Compares the vendored binaries in tools/ against the latest stable release of
    their upstream repository and optionally updates them.

.DESCRIPTION
    Pinned versions live in tools/tools.lock.json. For every tool the script:
      1. Lists the upstream releases and discards drafts, prereleases and any
         tag/name that looks like alpha/beta/rc.
      2. Picks the newest stable one by comparing version numbers (not dates: a
         patch for an older line published later must not win).
      3. If there is something new, downloads the asset and verifies its SHA256
         against the digest GitHub publishes for that asset, checks the PE
         architecture, runs a real functional test, and only then replaces the
         binary and updates the lock.

    It does NOT commit or open PRs: that is .github/workflows/update-tools.yml.
    Keeping them apart is what makes this runnable by hand.

.PARAMETER Id
    Tool id from the lock ('upx', '7zr') or 'all'.

.PARAMETER CheckOnly
    Only compare versions and report. Downloads nothing, changes nothing.

.PARAMETER SkipFunctionalTest
    Skip the functional test. For debugging only; the workflow never uses it.

.PARAMETER AllowMissingDigest
    Accept an asset for which GitHub publishes no digest. Off by default: a
    missing checksum is a downgrade in verification, so it has to be opted into
    explicitly rather than passing silently.

.PARAMETER GitHubToken
    Optional GitHub API token. Without one the limit is 60 requests/hour per IP,
    which shared runners burn through.

.PARAMETER PrBodyPath
    If set and an update happened, writes the PR markdown body there. The
    workflow feeds it to `gh pr create --body-file`. It is generated here and not
    in the YAML because a PowerShell here-string cannot live inside a YAML block
    scalar (the closing `"@` must sit at column 0).

.EXAMPLE
    pwsh tools/Update-VendoredTools.ps1 -CheckOnly
    Reports whether updates exist without touching the working tree.

.EXAMPLE
    pwsh tools/Update-VendoredTools.ps1 -Id upx
    Updates tools/upx.exe and tools/tools.lock.json if a newer stable exists.
#>
[CmdletBinding()]
param(
    [string] $Id = 'all',
    [string] $RepoRoot,
    [switch] $CheckOnly,
    [switch] $SkipFunctionalTest,
    [switch] $AllowMissingDigest,
    [string] $GitHubToken = $env:GITHUB_TOKEN,
    [string] $PrBodyPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

# Tags/names that are not stable. GitHub's `prerelease` flag is not enough on its
# own: neither upstream uses it, so a "26.03 beta" would arrive as a normal
# release.
$UnstablePattern = '(?i)(alpha|beta|preview|snapshot|nightly|(^|[^a-z])rc([^a-z]|\d|$))'

function Resolve-RepoRoot {
    param([string] $Provided)
    if ($Provided) { return (Resolve-Path $Provided).Path }
    return (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
}

function Get-LockPath { param([string] $Root) Join-Path $Root 'tools/tools.lock.json' }

function Read-Lock {
    param([string] $Root)
    $path = Get-LockPath $Root
    if (-not (Test-Path $path)) { throw "Lock file not found: $path" }
    return Get-Content $path -Raw -Encoding utf8 | ConvertFrom-Json
}

function Write-Lock {
    param([string] $Root, $Lock)
    # The whole file is reserialised. For the PR diff to show only the fields that
    # actually changed, the committed lock must be written in exactly this format
    # (see the idempotence note in tools/README.md).
    $json = ($Lock | ConvertTo-Json -Depth 10)
    $json = $json -replace "`r`n", "`n"
    Set-Content -Path (Get-LockPath $Root) -Value ($json + "`n") -Encoding utf8NoBOM -NoNewline
}

function Get-VersionSortKey {
    param([string] $Version)
    $parts = @($Version -split '[^0-9]+' | Where-Object { $_ -ne '' } | ForEach-Object { [int]$_ })
    if ($parts.Count -eq 0) { return '0000000000' }
    return (($parts | ForEach-Object { $_.ToString('0000000000') }) -join '.')
}

# Returns -1 if A < B, 0 if equal, 1 if A > B. Compares component by component, so
# it works for "5.1.1" (UPX) and "26.00" (7-Zip) alike.
function Compare-ToolVersion {
    param([string] $A, [string] $B)
    $pa = @($A -split '[^0-9]+' | Where-Object { $_ -ne '' } | ForEach-Object { [int]$_ })
    $pb = @($B -split '[^0-9]+' | Where-Object { $_ -ne '' } | ForEach-Object { [int]$_ })
    $n = [Math]::Max($pa.Count, $pb.Count)
    for ($i = 0; $i -lt $n; $i++) {
        $x = if ($i -lt $pa.Count) { $pa[$i] } else { 0 }
        $y = if ($i -lt $pb.Count) { $pb[$i] } else { 0 }
        if ($x -lt $y) { return -1 }
        if ($x -gt $y) { return 1 }
    }
    return 0
}

function Get-ApiHeaders {
    $h = @{ 'Accept' = 'application/vnd.github+json'; 'User-Agent' = 'RealViewOn-update-tools' }
    if ($GitHubToken) { $h['Authorization'] = "Bearer $GitHubToken" }
    return $h
}

function Get-LatestStableRelease {
    param([string] $Repo)
    $releases = Invoke-RestMethod -Uri "https://api.github.com/repos/$Repo/releases?per_page=50" -Headers (Get-ApiHeaders)
    $stable = @($releases | Where-Object {
        (-not $_.draft) -and (-not $_.prerelease) -and
        ($_.tag_name -notmatch $UnstablePattern) -and
        ([string]::IsNullOrEmpty($_.name) -or ($_.name -notmatch $UnstablePattern))
    })
    if ($stable.Count -eq 0) { throw "$Repo has no stable release in its 50 most recent entries." }
    return ($stable | Sort-Object -Property @{ Expression = { Get-VersionSortKey ($_.tag_name) } } -Descending)[0]
}

function Get-VersionFromTag {
    param([string] $Tag)
    return ($Tag -replace '^[vV]', '')
}

function Get-PeMachine {
    param([string] $Path)
    $fs = [System.IO.File]::OpenRead($Path)
    try {
        $br = New-Object System.IO.BinaryReader($fs)
        $fs.Position = 0x3C
        $peOffset = $br.ReadInt32()
        $fs.Position = $peOffset
        if ($br.ReadUInt32() -ne 0x00004550) { throw "$Path is not a valid PE image (missing 'PE' signature)." }
        switch ($br.ReadUInt16()) {
            0x8664  { 'x64' }
            0x014c  { 'x86' }
            0xAA64  { 'ARM64' }
            default { 'unknown' }
        }
    } finally { $fs.Dispose() }
}

function Get-Sha256 { param([string] $Path) (Get-FileHash -Path $Path -Algorithm SHA256).Hash }

# --- Functional tests --------------------------------------------------------
# A binary that downloads cleanly but cannot compress is useless. These tests use
# the same flags as the release pipeline, so the PR arrives already verified.

function Test-UpxBinary {
    param([string] $Exe, [string] $ExpectedVersion, [string] $PeSubject, [string] $WorkDir)
    $version = (& $Exe --version 2>&1) -join "`n"
    if ($version -notmatch [regex]::Escape($ExpectedVersion)) {
        throw "'upx --version' does not report $ExpectedVersion. Output: $version"
    }

    $copy = Join-Path $WorkDir 'subject.exe'
    Copy-Item -LiteralPath $PeSubject -Destination $copy -Force
    $before = (Get-Item $copy).Length

    # --ultra-brute is the flag ReleaseUPXZIP.bat and msbuild.yml use.
    & $Exe --ultra-brute -q $copy *> $null
    if ($LASTEXITCODE -ne 0) { throw "'upx --ultra-brute' failed with exit code $LASTEXITCODE." }
    $after = (Get-Item $copy).Length
    if ($after -ge $before) { throw "UPX did not shrink the executable ($before -> $after bytes)." }

    & $Exe -t $copy *> $null
    if ($LASTEXITCODE -ne 0) { throw "'upx -t' rejected the compressed executable." }

    # What actually matters: the compressed binary still runs.
    $output = (& $copy 2>&1) -join "`n"
    if ([string]::IsNullOrWhiteSpace($output)) { throw "The compressed executable produced no output when run." }

    $pct = [math]::Round(((($before - $after) / $before) * 100), 1)
    return "compressed the test subject $before -> $after bytes (-$pct%), 'upx -t' passed, and the compressed binary still runs"
}

function Test-7zrBinary {
    param([string] $Exe, [string] $ExpectedVersion, [string] $WorkDir)
    $banner = (& $Exe 2>&1) -join "`n"
    if ($banner -notmatch [regex]::Escape($ExpectedVersion)) {
        throw "The 7zr banner does not report $ExpectedVersion. Output: $banner"
    }

    $source = Join-Path $WorkDir 'source'
    New-Item -ItemType Directory -Force -Path $source | Out-Null
    Set-Content -Path (Join-Path $source 'text.txt') -Value ('RealViewOn ' * 800) -Encoding utf8
    Copy-Item -LiteralPath $Exe -Destination (Join-Path $source 'binary.exe') -Force
    $expected = Get-Sha256 (Join-Path $source 'binary.exe')

    # Same flags as the "Prepare release assets" step.
    $archive = Join-Path $WorkDir 'test.7z'
    & $Exe a -t7z -mx=9 -md=1m -ms=on $archive (Join-Path $source '*') *> $null
    if ($LASTEXITCODE -ne 0) { throw "'7zr a' failed with exit code $LASTEXITCODE." }
    if (-not (Test-Path $archive)) { throw "'7zr a' did not produce an archive." }

    & $Exe t $archive *> $null
    if ($LASTEXITCODE -ne 0) { throw "'7zr t' failed with exit code $LASTEXITCODE." }

    # Full round trip: extract and compare hashes rather than trusting exit codes.
    $dest = Join-Path $WorkDir 'extracted'
    & $Exe x $archive "-o$dest" -y *> $null
    if ($LASTEXITCODE -ne 0) { throw "'7zr x' failed with exit code $LASTEXITCODE." }
    $actual = Get-Sha256 (Join-Path $dest 'binary.exe')
    if ($actual -ne $expected) { throw "The round trip corrupted the data: $expected != $actual." }

    $size = (Get-Item $archive).Length
    return "built a $size byte .7z with the release flags, '7zr t' passed, and the extract+SHA256 round trip matches"
}

function Invoke-FunctionalTest {
    param($Tool, [string] $Exe, [string] $Version, [string] $Root, [string] $WorkDir)
    switch ($Tool.id) {
        'upx' {
            # Compresses the repo's own 7zr.exe: a real, small PE that is already here.
            $subject = Join-Path $Root 'tools/7zr.exe'
            if (-not (Test-Path $subject)) { throw "Test subject not found: $subject" }
            return Test-UpxBinary -Exe $Exe -ExpectedVersion $Version -PeSubject $subject -WorkDir $WorkDir
        }
        '7zr'   { return Test-7zrBinary -Exe $Exe -ExpectedVersion $Version -WorkDir $WorkDir }
        default { return "no functional test defined for '$($Tool.id)'" }
    }
}

# --- Download, checksum and replace ------------------------------------------

function Resolve-AssetName {
    param([string] $Template, [string] $Version)
    return ($Template -replace '\{version\}', $Version)
}

# GitHub publishes a per-asset digest ("sha256:<hex>") in the releases API. It is
# computed over the uploaded bytes, so checking the download against it detects
# corruption or tampering between the release and us. Note it covers the ASSET: for
# UPX that is the .zip, so this runs before extraction.
function Assert-AssetDigest {
    param($Asset, [string] $File, [string] $AssetName)

    $published = $null
    if ($Asset.PSObject.Properties['digest'] -and $Asset.digest) { $published = [string]$Asset.digest }

    if (-not $published) {
        # GitHub only computes digests for assets uploaded after the feature
        # shipped, so older releases have none.
        if (-not $AllowMissingDigest) {
            throw ("GitHub publishes no digest for '$AssetName', so its checksum cannot be verified.`n" +
                   "Re-run with -AllowMissingDigest to accept the asset unverified.")
        }
        Write-Host "    WARNING: no digest published for $AssetName; checksum NOT verified" -ForegroundColor Yellow
        return [pscustomobject]@{
            Verified = $false
            Sha256   = (Get-Sha256 $File)
            Note     = '**NOT VERIFIED** - GitHub publishes no digest for this asset, so the download could not be checked against an upstream hash.'
        }
    }

    if ($published -notmatch '^sha256:([0-9a-fA-F]{64})$') {
        throw "Unrecognised digest format for '$AssetName': '$published'. Expected 'sha256:<64 hex>'."
    }
    $expected = $Matches[1].ToUpperInvariant()
    $actual = Get-Sha256 $File

    if ($actual -ne $expected) {
        throw ("SHA256 mismatch on '$AssetName' - the download does not match what GitHub published.`n" +
               "  expected (GitHub digest): $expected`n" +
               "  actual   (downloaded)   : $actual")
    }

    Write-Host "    SHA256 matches the digest GitHub publishes for the asset" -ForegroundColor Green
    return [pscustomobject]@{
        Verified = $true
        Sha256   = $actual
        Note     = 'Verified against the SHA256 digest GitHub publishes for this asset.'
    }
}

function Get-BinaryFromRelease {
    param($Tool, $Release, [string] $Version, [string] $WorkDir)
    $assetName = Resolve-AssetName $Tool.asset $Version
    $asset = $Release.assets | Where-Object { $_.name -eq $assetName } | Select-Object -First 1
    if (-not $asset) {
        $available = ($Release.assets | ForEach-Object { $_.name }) -join ', '
        throw "Release $($Release.tag_name) of $($Tool.repo) does not publish '$assetName'. Available assets: $available"
    }

    $download = Join-Path $WorkDir $assetName
    Write-Host "    downloading $assetName ($([math]::Round($asset.size / 1KB, 1)) KB)"
    Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $download -UseBasicParsing -Headers @{ 'User-Agent' = 'RealViewOn-update-tools' }

    # Checksum first: nothing gets extracted or executed before this passes.
    $digest = Assert-AssetDigest -Asset $asset -File $download -AssetName $assetName

    $result = [pscustomobject]@{
        Binary         = $download
        AssetUrl       = $asset.browser_download_url
        AssetName      = $assetName
        AssetSha256    = $digest.Sha256
        DigestVerified = $digest.Verified
        DigestNote     = $digest.Note
    }

    if ($Tool.archiveMember) {
        $member = Resolve-AssetName $Tool.archiveMember $Version
        $extracted = Join-Path $WorkDir 'zip'
        Expand-Archive -LiteralPath $download -DestinationPath $extracted -Force
        $path = Join-Path $extracted ($member -replace '/', [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path $path)) {
            $contents = (Get-ChildItem $extracted -Recurse -File | ForEach-Object { $_.FullName.Substring($extracted.Length + 1) }) -join ', '
            throw "The asset does not contain '$member'. Contents: $contents"
        }
        $result.Binary = $path
    }

    return $result
}

function Invoke-ToolCheck {
    param($Tool, [string] $Root)

    Write-Host "==> $($Tool.name) [$($Tool.id)]"
    $localPath = Join-Path $Root $Tool.path
    if (-not (Test-Path $localPath)) { throw "Vendored binary not found: $localPath" }

    # The lock is the reference, but if it disagrees with the binary on disk then
    # someone changed it by hand and nothing downstream can be trusted.
    $localSha = Get-Sha256 $localPath
    if ($Tool.sha256 -and ($localSha -ne $Tool.sha256)) {
        throw ("SHA256 of $($Tool.path) does not match tools.lock.json.`n" +
               "  on disk : $localSha`n" +
               "  in lock : $($Tool.sha256)`n" +
               "Update the lock by hand before letting the bot manage this tool.")
    }

    $release = Get-LatestStableRelease -Repo $Tool.repo
    $latest = Get-VersionFromTag $release.tag_name
    $current = $Tool.version
    $cmp = Compare-ToolVersion -A $latest -B $current

    Write-Host "    pinned: $current    latest stable: $latest ($($release.tag_name))"

    $result = [pscustomobject]@{
        Id             = $Tool.id
        Name           = $Tool.name
        Path           = $Tool.path
        Repo           = $Tool.repo
        CurrentVersion = $current
        LatestVersion  = $latest
        Tag            = $release.tag_name
        ReleaseUrl     = $release.html_url
        # Fixed format: ConvertFrom-Json hands back a DateTime and casting it to
        # string would use the runner's culture.
        PublishedAt    = ([datetime]$release.published_at).ToUniversalTime().ToString('yyyy-MM-dd')
        HasUpdate      = ($cmp -gt 0)
        OldSha256      = $localSha
        NewSha256      = $null
        AssetSha256    = $null
        DigestVerified = $false
        DigestNote     = $null
        OldSize        = (Get-Item $localPath).Length
        NewSize        = $null
        AssetName      = $null
        AssetUrl       = $null
        Arch           = $Tool.arch
        Verification   = $null
        Updated        = $false
    }

    if ($cmp -lt 0) {
        Write-Host "    the pinned version is NEWER than the latest published stable; leaving it alone." -ForegroundColor Yellow
        return $result
    }
    if ($cmp -eq 0) {
        Write-Host "    up to date." -ForegroundColor Green
        return $result
    }

    Write-Host "    update available: $current -> $latest" -ForegroundColor Cyan
    if ($CheckOnly) { return $result }

    $workDir = Join-Path ([System.IO.Path]::GetTempPath()) ("rvo-tools-" + $Tool.id + "-" + [System.Guid]::NewGuid().ToString('N').Substring(0, 8))
    New-Item -ItemType Directory -Force -Path $workDir | Out-Null
    try {
        $download = Get-BinaryFromRelease -Tool $Tool -Release $release -Version $latest -WorkDir $workDir
        $newBinary = $download.Binary
        $result.AssetSha256 = $download.AssetSha256
        $result.DigestVerified = $download.DigestVerified
        $result.DigestNote = $download.DigestNote
        $result.AssetName = $download.AssetName
        $result.AssetUrl = $download.AssetUrl

        # Architecture: without this an upstream asset rename could slip an x86 in
        # where an x64 used to be, and nobody would spot it in a binary diff.
        $arch = Get-PeMachine $newBinary
        if ($arch -ne $Tool.arch) {
            throw "Unexpected architecture in $($download.AssetName): expected $($Tool.arch) but found $arch."
        }

        if ($SkipFunctionalTest) {
            $result.Verification = 'SKIPPED (-SkipFunctionalTest)'
            Write-Host "    functional test SKIPPED" -ForegroundColor Yellow
        } else {
            $result.Verification = Invoke-FunctionalTest -Tool $Tool -Exe $newBinary -Version $latest -Root $Root -WorkDir $workDir
            Write-Host "    functional test passed: $($result.Verification)" -ForegroundColor Green
        }

        $result.NewSha256 = Get-Sha256 $newBinary
        $result.NewSize = (Get-Item $newBinary).Length

        Copy-Item -LiteralPath $newBinary -Destination $localPath -Force
        $Tool.version = $latest
        $Tool.sha256 = $result.NewSha256
        $result.Updated = $true
        Write-Host "    updated $($Tool.path)" -ForegroundColor Green
    } finally {
        Remove-Item -LiteralPath $workDir -Recurse -Force -ErrorAction SilentlyContinue
    }

    return $result
}

# --- PR body -----------------------------------------------------------------
# Single-quoted here-string template: no interpolation, so markdown backticks stay
# literal and need no escaping.

function New-PrBody {
    param($Result, $Tool)

    $template = @'
## {NAME} `{OLD}` -> `{NEW}`

Automated update of a vendored binary, detected by
[`update-tools.yml`](.github/workflows/update-tools.yml).

|         | Before      | After       |
| ---     | ---         | ---         |
| Version | `{OLD}`     | `{NEW}`     |
| SHA256  | `{OLD_SHA}` | `{NEW_SHA}` |
| Size    | {OLD_SIZE} bytes | {NEW_SIZE} bytes |

- **Upstream release:** {RELEASE_URL} (tag `{TAG}`, published {PUBLISHED})
- **Asset downloaded:** [`{ASSET}`]({ASSET_URL})
- **Architecture:** `{ARCH}`, read from the PE header. Matches the value pinned in the lock.
- **Used by:** {USED_BY}

### Checksum

{DIGEST_NOTE}

Asset SHA256: `{ASSET_SHA}`

### Automated verification

The bot did not just download the binary: it ran a functional test using the same
flags as the release pipeline.

> {VERIFICATION}

### What to check by hand

This PR replaces an executable, so **the diff is not reviewable**. The checksum
above was verified automatically against the digest GitHub publishes for the
asset, which detects corruption or tampering in transit. It does not by itself
prove who built the binary, so if you want an independent confirmation compare it
against the vendor's own published hash:

- UPX: <https://github.com/upx/upx/releases>
- 7-Zip: <https://www.7-zip.org/download.html>

---
This PR was created by `GITHUB_TOKEN`, so it **does not trigger other workflows**.
'@

    $map = [ordered]@{
        '{NAME}'         = [string]$Result.Name
        '{OLD}'          = [string]$Result.CurrentVersion
        '{NEW}'          = [string]$Result.LatestVersion
        '{OLD_SHA}'      = [string]$Result.OldSha256
        '{NEW_SHA}'      = [string]$Result.NewSha256
        '{OLD_SIZE}'     = [string]$Result.OldSize
        '{NEW_SIZE}'     = [string]$Result.NewSize
        '{RELEASE_URL}'  = [string]$Result.ReleaseUrl
        '{TAG}'          = [string]$Result.Tag
        '{PUBLISHED}'    = [string]$Result.PublishedAt
        '{ASSET}'        = [string]$Result.AssetName
        '{ASSET_URL}'    = [string]$Result.AssetUrl
        '{ASSET_SHA}'    = [string]$Result.AssetSha256
        '{DIGEST_NOTE}'  = [string]$Result.DigestNote
        '{ARCH}'         = [string]$Result.Arch
        '{USED_BY}'      = [string]$Tool.usedBy
        '{VERIFICATION}' = [string]$Result.Verification
    }

    $md = $template
    foreach ($key in $map.Keys) { $md = $md.Replace($key, $map[$key]) }
    return $md
}

# --- Main --------------------------------------------------------------------

$root = Resolve-RepoRoot $RepoRoot
$lock = Read-Lock $root

# The outer @() matters: an if branch returning an empty array collapses to $null
# on assignment, and with Set-StrictMode the .Count below then blows up with an
# error that explains nothing.
$selected = @(if ($Id -eq 'all') { $lock.tools } else { $lock.tools | Where-Object { $_.id -eq $Id } })
if ($selected.Count -eq 0) {
    $valid = (@($lock.tools) | ForEach-Object { $_.id }) -join ', '
    throw "No tool with id '$Id' in tools/tools.lock.json. Valid ids: $valid"
}

$results = @()
foreach ($tool in $selected) {
    $r = Invoke-ToolCheck -Tool $tool -Root $root
    $results += $r
    if ($r.Updated -and $PrBodyPath) {
        New-PrBody -Result $r -Tool $tool | Set-Content -Path $PrBodyPath -Encoding utf8NoBOM
        Write-Host "    PR body written to $PrBodyPath"
    }
}

if (@($results | Where-Object { $_.Updated }).Count -gt 0) { Write-Lock -Root $root -Lock $lock }

# Outputs for GitHub Actions (the workflow runs one id per job).
if ($env:GITHUB_OUTPUT -and $results.Count -eq 1) {
    $r = $results[0]
    $pairs = [ordered]@{
        updated         = $r.Updated.ToString().ToLower()
        has_update      = $r.HasUpdate.ToString().ToLower()
        digest_verified = $r.DigestVerified.ToString().ToLower()
        current_version = $r.CurrentVersion
        latest_version  = $r.LatestVersion
        tag             = $r.Tag
        release_url     = [string]$r.ReleaseUrl
        published_at    = [string]$r.PublishedAt
        asset_name      = [string]$r.AssetName
        asset_url       = [string]$r.AssetUrl
        asset_sha256    = [string]$r.AssetSha256
        sha256_old      = [string]$r.OldSha256
        sha256_new      = [string]$r.NewSha256
        size_old        = [string]$r.OldSize
        size_new        = [string]$r.NewSize
        arch            = [string]$r.Arch
        verification    = [string]$r.Verification
        name            = [string]$r.Name
        repo            = [string]$r.Repo
        path            = [string]$r.Path
    }
    # GITHUB_OUTPUT is "key=value" per line: a newline inside a value breaks the
    # format (and is a known injection vector). Every value here is single-line
    # already, but they are flattened just in case.
    foreach ($k in $pairs.Keys) {
        $v = ([string]$pairs[$k]) -replace '[\r\n]+', ' '
        "$k=$v" | Out-File -FilePath $env:GITHUB_OUTPUT -Append -Encoding utf8
    }
}

Write-Host ''
$results | Format-Table Id, CurrentVersion, LatestVersion, HasUpdate, DigestVerified, Updated -AutoSize
return $results
