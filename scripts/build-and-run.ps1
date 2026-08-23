<#
.SYNOPSIS
  Configures, builds, and runs the MU client via the CMake presets in
  CMakePresets.json, from a plain PowerShell prompt (no Developer Command
  Prompt required -- this script imports the VC dev environment itself).

.EXAMPLE
  ./scripts/build-and-run.ps1
  Build+run the x64 Debug client, blocking until it exits (console attached).

.EXAMPLE
  ./scripts/build-and-run.ps1 -Detached
  Build the x64 Debug client, then launch it detached and return immediately.

.EXAMPLE
  ./scripts/build-and-run.ps1 -Arch x86 -Config Release -Editor
  Build+run the x86 Release MuEditor client.

.EXAMPLE
  ./scripts/build-and-run.ps1 -NoBuild -Detached
  Skip the build, just (re)launch the already-built exe detached.

.EXAMPLE
  ./scripts/build-and-run.ps1 -- connect /u192.168.0.20 /p55902
  Pass extra arguments through to Main.exe (after --).
#>
[CmdletBinding()]
param(
    [ValidateSet('x64', 'x86')]
    [string]$Arch = 'x64',

    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Debug',

    [switch]$Editor,
    [switch]$Clean,
    [switch]$NoBuild,
    [switch]$Detached,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ClientArgs = @()
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

function Import-VcVars {
    param([string]$Arch)

    $vswhereCandidates = @(
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
        "${env:ProgramFiles}\Microsoft Visual Studio\Installer\vswhere.exe"
    )
    $vswhere = $vswhereCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $vswhere) {
        throw 'vswhere.exe not found. Install Visual Studio with the C++ workload.'
    }

    $vsInstallPath = & $vswhere -latest -prerelease -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath
    if (-not $vsInstallPath) {
        throw 'No Visual Studio installation with the C++ Tools component was found.'
    }

    $vcvarsall = Join-Path $vsInstallPath 'VC\Auxiliary\Build\vcvarsall.bat'
    if (-not (Test-Path $vcvarsall)) {
        throw "vcvarsall.bat not found at '$vcvarsall'."
    }

    # x86 is cross-compiled from the x64 host toolchain (x64_x86) rather than
    # requiring a separate native x86 host install.
    $archArg = if ($Arch -eq 'x64') { 'x64' } else { 'x64_x86' }

    # Run vcvarsall in a child cmd.exe, then dump its resulting environment so
    # we can import it into *this* PowerShell process -- everything launched
    # afterwards (cmake, ninja, cl, link, dotnet publish) inherits it.
    $envDump = cmd /c "call `"$vcvarsall`" $archArg >nul 2>&1 && set"
    if ($LASTEXITCODE -ne 0 -or -not $envDump) {
        throw "vcvarsall.bat failed for arch '$archArg'."
    }
    foreach ($line in $envDump) {
        if ($line -match '^([^=]+)=(.*)$') {
            [System.Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], 'Process')
        }
    }
}

$presetBase = "windows-$Arch" + $(if ($Editor) { '-mueditor' } else { '' })
$buildPreset = "$presetBase-$($Config.ToLower())"
$buildDir = Join-Path $repoRoot "out\build\$presetBase"
$exeDir = Join-Path $buildDir "src\$Config"
$exePath = Join-Path $exeDir 'Main.exe'

if (-not $NoBuild) {
    if ($Clean -and (Test-Path $buildDir)) {
        Write-Host "Removing $buildDir ..."
        Remove-Item -Recurse -Force $buildDir
    }

    Write-Host "Setting up VC environment for $Arch ..."
    Import-VcVars -Arch $Arch

    Push-Location $repoRoot
    try {
        Write-Host "Configuring preset '$presetBase' ..."
        cmake --preset $presetBase
        if ($LASTEXITCODE -ne 0) { throw 'cmake configure failed.' }

        Write-Host "Building preset '$buildPreset' ..."
        cmake --build --preset $buildPreset
        if ($LASTEXITCODE -ne 0) { throw 'cmake build failed.' }
    }
    finally {
        Pop-Location
    }
}

if (-not (Test-Path $exePath)) {
    throw "Main.exe not found at '$exePath'. Run without -NoBuild first."
}

$launchArgs = @()
if ($Editor) { $launchArgs += '--editor' }
$launchArgs += $ClientArgs

if ($Detached) {
    $startProcessArgs = @{
        FilePath         = $exePath
        WorkingDirectory = $exeDir
        PassThru         = $true
    }
    if ($launchArgs.Count -gt 0) { $startProcessArgs['ArgumentList'] = $launchArgs }
    $proc = Start-Process @startProcessArgs
    Write-Host "Launched detached: $exePath (PID $($proc.Id))"
}
else {
    Push-Location $exeDir
    try {
        & $exePath @launchArgs
    }
    finally {
        Pop-Location
    }
}
