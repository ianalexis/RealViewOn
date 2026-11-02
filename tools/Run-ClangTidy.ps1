#Requires -Version 7.0
<#
.SYNOPSIS
    Runs clang-tidy, with the checks in .clang-tidy, over RealViewOn's own code. It is
    the check that gates pull requests (.github/workflows/clang-tidy.yml).

.DESCRIPTION
    1. Asks MSBuild for the command line of every file RealViewOn.vcxproj,
       tests/RealViewOn.Tests.vcxproj and RealViewOnGUI.vcxproj compile (the
       GetCompileCommands target that Visual Studio's own clang-tidy integration uses)
       and writes them to
       <BuildDir>/compile_commands.json. clang-tidy therefore parses with exactly the
       flags of the build, and a flag changed in a .vcxproj needs no change here.
    2. Installs the clang-tidy pinned in tools/clang_tidy_requirements.txt into a
       virtual environment in <BuildDir>, unless -ClangTidy names one. CI uses the
       same pin, so a local run reports what CI reports.
    3. Runs it on every .cpp and .h under src/, include/ and tests/, except the
       vendored tests/doctest.h. Headers are checked on their own, because
       misc-include-cleaner only reports on the file it was given.

    Whole files are checked, not only the lines a change touches: the code base
    passes, so any finding is new.

.PARAMETER Fix
    Apply clang-tidy's fixes (adds the missing includes), then report what is left.

.PARAMETER ClangTidy
    clang-tidy executable to use instead of the pinned one.

.PARAMETER BuildDir
    Where the compile database and the virtual environment go. Default: build-tidy.

.PARAMETER Jobs
    Parallel clang-tidy runs. Default: one per core.

.EXAMPLE
    pwsh tools/Run-ClangTidy.ps1
    Reports every finding, exits 1 if there is any.

.EXAMPLE
    pwsh tools/Run-ClangTidy.ps1 -Fix
    Adds the missing includes, then reports whatever needs a manual fix.
#>
[CmdletBinding()]
param(
    [switch] $Fix,
    [string] $ClangTidy = $env:CLANG_TIDY,
    [string] $BuildDir = 'build-tidy',
    [int] $Jobs = [Environment]::ProcessorCount
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root
if (-not [System.IO.Path]::IsPathRooted($BuildDir)) { $BuildDir = Join-Path $Root $BuildDir }
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

$Projects = @('RealViewOn.vcxproj', 'tests\RealViewOn.Tests.vcxproj', 'RealViewOnGUI.vcxproj')
$CheckedDirs = @('src', 'include', 'tests')
$Excluded = @('tests\doctest.h')

$files = Get-ChildItem -Path $CheckedDirs -Recurse -File -Include '*.cpp', '*.h' |
    ForEach-Object { [System.IO.Path]::GetRelativePath($Root, $_.FullName) } |
    Where-Object { $_ -notin $Excluded -and $_ -notmatch '^tests\\x64\\' } |
    Sort-Object

# --- Compile database ---------------------------------------------------------

$Vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $Vswhere)) { throw 'Visual Studio is not installed (vswhere.exe not found).' }
$MSBuild = & $Vswhere -latest -prerelease -products * -requires Microsoft.Component.MSBuild `
    -find 'MSBuild\**\Bin\amd64\MSBuild.exe' | Select-Object -First 1
if (-not $MSBuild) { throw 'MSBuild not found. Install Visual Studio with the C++ workload.' }

# A file both projects compile keeps the first project's command: the shipped build,
# which also turns warnings into errors.
$entries = [ordered]@{}
$projectCommand = @{}
foreach ($project in $Projects) {
    # -getTargetResult prints the target's items as JSON instead of the build log.
    $json = & $MSBuild $project -nologo -t:GetCompileCommands -p:Configuration=Release `
        -p:Platform=x64 -getTargetResult:GetCompileCommands
    if ($LASTEXITCODE -ne 0) { $json | Write-Host; throw "MSBuild could not list the compile commands of $project." }
    # One item per set of files sharing a command line: Identity holds the flags. clang-tidy
    # only parses, so /c goes unused, which /WX would turn into an error.
    foreach ($item in ($json | ConvertFrom-Json).TargetResults.GetCompileCommands.Items) {
        $flags = $item.Identity -replace '(^|\s)/c(?=\s)', ''
        $projectCommand[$project] ??= @{ Directory = $item.WorkingDirectory; Flags = $flags }
        foreach ($file in $item.Files -split ';') {
            if ($entries.Contains($file)) { continue }
            $entries[$file] = [ordered]@{
                directory = $item.WorkingDirectory
                file      = $file
                command   = "clang-cl.exe $flags `"$file`""
            }
        }
    }
}
# MSBuild lists only sources. A header gets the command of the project that compiles the
# code around it: clang-tidy's own guess drops /std. Parsed as the main file, a header's
# #pragma once would warn.
foreach ($header in $files -like '*.h') {
    $owner = if ($header -like 'tests\*') { 'tests\RealViewOn.Tests.vcxproj' }
             elseif ($header -like 'src\gui\*') { 'RealViewOnGUI.vcxproj' }
             else { 'RealViewOn.vcxproj' }
    $command = $projectCommand[$owner]
    $file = Join-Path $Root $header
    $entries[$file] = [ordered]@{
        directory = $command.Directory
        file      = $file
        command   = "clang-cl.exe $($command.Flags) -Wno-pragma-once-outside-header `"$file`""
    }
}
@($entries.Values) | ConvertTo-Json -Depth 3 -AsArray | Set-Content -Encoding utf8 (Join-Path $BuildDir 'compile_commands.json')

# --- clang-tidy ---------------------------------------------------------------

$Requirements = Join-Path $PSScriptRoot 'clang_tidy_requirements.txt'
$Pinned = (Select-String -Path $Requirements -Pattern '^clang-tidy==(.+)$').Matches[0].Groups[1].Value.Trim()

if (-not $ClangTidy) {
    $Venv = Join-Path $BuildDir 'clang-tidy-venv'
    $ClangTidy = Join-Path $Venv 'Scripts\clang-tidy.exe'
    if (-not ((Test-Path $ClangTidy) -and ((& $ClangTidy --version) -match "version $([regex]::Escape($Pinned))"))) {
        $Python = if (Get-Command py -ErrorAction SilentlyContinue) { @('py', '-3') }
                  elseif (Get-Command python -ErrorAction SilentlyContinue) { @('python') }
                  else { throw 'Python 3 is needed to install clang-tidy: winget install -e --id Python.Python.3.12' }
        Write-Host "Installing clang-tidy $Pinned into $Venv"
        & $Python[0] @($Python | Select-Object -Skip 1) -m venv $Venv
        if ($LASTEXITCODE -ne 0) { throw 'Could not create a Python virtual environment.' }
        & (Join-Path $Venv 'Scripts\python.exe') -m pip install --quiet --disable-pip-version-check -r $Requirements
        if ($LASTEXITCODE -ne 0) { throw "Installing clang-tidy $Pinned failed." }
    }
}
$version = (& $ClangTidy --version) -join ' '
if ($version -notmatch "version $([regex]::Escape($Pinned))") {
    Write-Warning "$ClangTidy is not clang-tidy $Pinned, so results may differ from CI."
}

# --- Run ----------------------------------------------------------------------

function Invoke-ClangTidy([string[]] $ExtraArgs) {
    $tidyArgs = @('-p', $BuildDir, '--quiet') + $ExtraArgs
    $files | ForEach-Object -ThrottleLimit $Jobs -Parallel {
        $output = & $using:ClangTidy @using:tidyArgs $_ 2>&1 | ForEach-Object { "$_" }
        [pscustomobject]@{ File = $_; ExitCode = $LASTEXITCODE; Output = $output }
    }
}

if ($Fix) {
    Write-Host "Fixing $($files.Count) file(s) with $version"
    Invoke-ClangTidy @('--fix') | Out-Null
    # clang-tidy writes the lines it inserts with LF. Latin1 maps every byte to one
    # character, so the rest of the file, BOM included, is written back unchanged.
    foreach ($file in $files) {
        $text = [System.IO.File]::ReadAllText((Join-Path $Root $file), [System.Text.Encoding]::Latin1)
        if ($text.Contains("`r`n") -and $text -match '(?<!\r)\n') {
            [System.IO.File]::WriteAllText((Join-Path $Root $file), ($text -replace '(?<!\r)\n', "`r`n"), [System.Text.Encoding]::Latin1)
        }
    }
}

# After -Fix, checked again, so what is reported is what the fixes left.
Write-Host "Checking $($files.Count) file(s) with $version"
$results = Invoke-ClangTidy @()

$annotate = $env:GITHUB_ACTIONS -eq 'true'
$failed = @($results | Where-Object ExitCode -ne 0 | Sort-Object File)
foreach ($result in $failed) {
    Write-Host "`n==== $($result.File)"
    foreach ($line in $result.Output) {
        Write-Host $line
        if ($annotate -and $line -match '^(.+?):(\d+):(\d+): (error|warning): (.*)$') {
            $file = [System.IO.Path]::GetRelativePath($Root, $Matches[1]) -replace '\\', '/'
            Write-Host "::$($Matches[4]) file=$file,line=$($Matches[2]),col=$($Matches[3])::$($Matches[5])"
        }
    }
}

if ($failed) {
    $hint = if ($Fix) { 'What is listed above is what -Fix could not fix.' }
            else { 'Add the includes it names, or let pwsh tools/Run-ClangTidy.ps1 -Fix add them. Other findings need a manual fix.' }
    Write-Host "`nclang-tidy failed on $($failed.Count) file(s). $hint"
    exit 1
}
Write-Host 'clang-tidy passed.'
