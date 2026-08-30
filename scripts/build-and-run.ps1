<#
.SYNOPSIS
  Configures, builds, and runs the MU client on this branch via the CMake
  presets in CMakePresets.json, from a plain PowerShell prompt (no Developer
  Command Prompt required -- this script imports the VC dev environment
  itself, same as the feature/rmlui-migration script it's modeled on).

  Unlike that script, this branch's build additionally needs:
    - vcpkg chain-loaded in (openssl/curl/directx-dxc/glslang/spirv-cross
      aren't found otherwise -- see vcpkg.json). Set $env:VCPKG_ROOT or pass
      -VcpkgRoot; defaults to C:\tmp\vcpkg if present.
    - the VS Installer directory on PATH, or the .NET Native AOT build
      (MUnique.Client.Library.csproj) fails calling vswhere.exe internally.
      vcvarsall.bat alone doesn't put it there.
    - a runtime GPU backend choice, via $env:SDL_GPU_DRIVER (SDL_GPU has no
      OpenGL backend on this branch -- it's vulkan/direct3d12/metal only).

.EXAMPLE
  ./scripts/build-and-run.ps1
  Build+run the x64 Debug client (default SDL_GPU backend), blocking until
  it exits.

.EXAMPLE
  ./scripts/build-and-run.ps1 -Config Release -GpuDriver direct3d12 -Detached
  Build the x64 Release client forced onto the D3D12 backend, then launch
  it detached and return immediately.

.EXAMPLE
  ./scripts/build-and-run.ps1 -Arch x86 -Config Release -Editor
  Build+run the x86 Release MuEditor client.

.EXAMPLE
  ./scripts/build-and-run.ps1 -NoBuild -GpuDriver vulkan -Detached
  Skip the build, just (re)launch the already-built exe forced onto Vulkan.

.EXAMPLE
  ./scripts/build-and-run.ps1 -- connect /u192.168.0.20 /p55902
  Pass extra arguments through to Main.exe (after --).
#>
[CmdletBinding()]
param(
    [ValidateSet('x64', 'x86')]
    [string]$Arch = 'x64',

    [ValidateSet('Debug', 'Release', 'RelWithDebInfo')]
    [string]$Config = 'Debug',

    [ValidateSet('vulkan', 'direct3d12', 'metal')]
    [string]$GpuDriver,

    [string]$VcpkgRoot,

    [ValidateSet('x64-windows-static-md', 'x64-windows')]
    [string]$VcpkgTriplet = 'x64-windows-static-md',

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

    # vcvarsall alone doesn't put the VS Installer directory (vswhere.exe) on
    # PATH. The .NET Native AOT link step for MUnique.Client.Library.csproj
    # shells out to vswhere internally to find link.exe -- without this it
    # fails with "'vswhere.exe' is not recognized" partway through the build.
    $installerDir = Split-Path -Parent $vswhere
    if (($env:PATH -split ';') -notcontains $installerDir) {
        $env:PATH = "$installerDir;$env:PATH"
    }
}

function Resolve-VcpkgRoot {
    param([string]$Explicit)

    if ($Explicit) { return $Explicit }
    if ($env:VCPKG_ROOT) { return $env:VCPKG_ROOT }

    $candidates = @('C:\tmp\vcpkg', "$env:USERPROFILE\vcpkg", 'C:\vcpkg')
    $found = $candidates | Where-Object { Test-Path (Join-Path $_ 'vcpkg.exe') } | Select-Object -First 1
    if ($found) { return $found }

    throw ('vcpkg not found. Pass -VcpkgRoot <path>, set $env:VCPKG_ROOT, or clone+bootstrap ' +
           'vcpkg (git clone https://github.com/microsoft/vcpkg && vcpkg\bootstrap-vcpkg.bat) ' +
           'to build openssl/curl/directx-dxc/glslang/spirv-cross (see vcpkg.json).')
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

    $vcpkgRootResolved = Resolve-VcpkgRoot -Explicit $VcpkgRoot
    $vcpkgToolchain = Join-Path $vcpkgRootResolved 'scripts\buildsystems\vcpkg.cmake'
    if (-not (Test-Path $vcpkgToolchain)) {
        throw "vcpkg toolchain not found at '$vcpkgToolchain' -- is '$vcpkgRootResolved' bootstrapped?"
    }
    $env:VCPKG_ROOT = $vcpkgRootResolved
    Write-Host "Using vcpkg at $vcpkgRootResolved (triplet: $VcpkgTriplet) ..."

    Push-Location $repoRoot
    try {
        # CMAKE_TOOLCHAIN_FILE sticks to a build tree after its first configure --
        # -Clean (or a first-ever configure) is required to actually change it.
        Write-Host "Configuring preset '$presetBase' ..."
        cmake --preset $presetBase `
            "-DCMAKE_TOOLCHAIN_FILE=$($vcpkgToolchain -replace '\\','/')" `
            "-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=$($repoRoot -replace '\\','/')/toolchain-$Arch.cmake" `
            "-DVCPKG_TARGET_TRIPLET=$VcpkgTriplet"
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

if ($GpuDriver) {
    Write-Host "Forcing SDL_GPU_DRIVER=$GpuDriver ..."
    $env:SDL_GPU_DRIVER = $GpuDriver
}
else {
    Remove-Item Env:SDL_GPU_DRIVER -ErrorAction SilentlyContinue
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
