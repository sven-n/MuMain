# MuMain Build and Configure Script (Windows 11)
# Configures, builds all four Release variants, and sets config.ini with server IP/port.
#
# Usage: .\build-and-configure.ps1
# Optional: .\build-and-configure.ps1 -ServerIP "192.168.3.34" -ServerPort 44406

param(
    [string]$ServerIP = "192.168.3.34",
    [int]$ServerPort = 44406
)

$ErrorActionPreference = "Stop"
$RepoRoot = $PSScriptRoot

# Ensure we're in repo root
Set-Location $RepoRoot

# Find cmake if not in PATH
$cmakeExe = $null
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    $cmakeExe = "cmake"
} else {
    $searchPaths = @(
        "C:\Program Files\CMake\bin\cmake.exe",
        "C:\Program Files (x86)\CMake\bin\cmake.exe"
    )
    # Visual Studio 2022 / 2019 (Community, Professional, Enterprise, BuildTools)
    $vsEditions = @("Community", "Professional", "Enterprise", "BuildTools")
    foreach ($ver in @("2022", "2019")) {
        foreach ($ed in $vsEditions) {
            $searchPaths += "C:\Program Files\Microsoft Visual Studio\$ver\$ed\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        }
    }
    foreach ($p in $searchPaths) {
        if ($p -and (Test-Path $p)) { $cmakeExe = $p; break }
    }
}
if (-not $cmakeExe) {
    Write-Host "CMake not found. Install CMake or run from 'Developer PowerShell for VS 2022'." -ForegroundColor Red
    Write-Host "  Start Menu -> Visual Studio 2022 -> Developer PowerShell for VS 2022" -ForegroundColor Gray
    exit 1
}

Write-Host "=== MuMain Build & Configure ===" -ForegroundColor Cyan
Write-Host "Server: $ServerIP : $ServerPort" -ForegroundColor Gray
Write-Host ""

function Invoke-Build {
    param([string]$PresetName, [string]$BuildPreset)
    Write-Host "[$PresetName] Configuring..." -ForegroundColor Yellow
    & $cmakeExe --preset $PresetName
    if ($LASTEXITCODE -ne 0) { throw "Configure failed: $PresetName" }
    Write-Host "[$PresetName] Building..." -ForegroundColor Yellow
    & $cmakeExe --build --preset $BuildPreset
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $BuildPreset" }
    Write-Host "[$PresetName] Done." -ForegroundColor Green
}

# Build all four Release variants
Invoke-Build -PresetName "windows-x86" -BuildPreset "windows-x86-release"
Invoke-Build -PresetName "windows-x64" -BuildPreset "windows-x64-release"
Invoke-Build -PresetName "windows-x86-mueditor" -BuildPreset "windows-x86-mueditor-release"
Invoke-Build -PresetName "windows-x64-mueditor" -BuildPreset "windows-x64-mueditor-release"

# Output directories (config.ini lives next to Main.exe)
$OutputDirs = @(
    "out\build\windows-x86\src\Release",
    "out\build\windows-x64\src\Release",
    "out\build\windows-x86-mueditor\src\Release",
    "out\build\windows-x64-mueditor\src\Release"
)

$ConfigContent = @"
[CONNECTION SETTINGS]
ServerIP=$ServerIP
ServerPort=$ServerPort

"@

Write-Host ""
Write-Host "=== Configuring config.ini ===" -ForegroundColor Cyan
foreach ($dir in $OutputDirs) {
    $configPath = Join-Path $RepoRoot (Join-Path $dir "config.ini")
    $dirPath = Split-Path $configPath -Parent
    if (Test-Path $dirPath) {
        Set-Content -Path $configPath -Value $ConfigContent -Encoding UTF8
        Write-Host "  $configPath" -ForegroundColor Gray
    } else {
        Write-Warning "Output dir not found: $dirPath"
    }
}

Write-Host ""
Write-Host "=== Complete ===" -ForegroundColor Green
Write-Host "Main.exe built in:"
foreach ($dir in $OutputDirs) {
    $exePath = Join-Path $RepoRoot (Join-Path $dir "Main.exe")
    if (Test-Path $exePath) {
        Write-Host "  $exePath"
    }
}
Write-Host ""
Write-Host "Run: Main.exe (from its directory) or use connect params: Main.exe connect /u$ServerIP /p$ServerPort"
