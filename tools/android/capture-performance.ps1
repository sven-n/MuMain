param(
    [int]$DurationSeconds = 120,
    [string]$Device,
    [string]$OutputPath
)

$ErrorActionPreference = 'Stop'

$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
if (-not $OutputPath) {
    $timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $OutputPath = Join-Path $repositoryRoot "out\android\captures\mumain-$timestamp.log"
}
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutputPath) | Out-Null

$adbArguments = @()
if ($Device) {
    $adbArguments += @('-s', $Device)
}

& adb @adbArguments get-state | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw 'No usable Android device is connected.'
}

& adb @adbArguments logcat -c
& adb @adbArguments shell am start -n com.alin.mumain/.LoadingActivity | Out-Null

$logProcess = Start-Process adb `
    -ArgumentList ($adbArguments + @(
        'logcat', '-v', 'threadtime',
        'MuMainFPS:I', 'MuMainGL:I', 'MuMainPace:I', 'MuMainInput:I', '*:S')) `
    -RedirectStandardOutput $OutputPath `
    -NoNewWindow `
    -PassThru

try {
    Write-Host "Capturing renderer telemetry for $DurationSeconds seconds to $OutputPath"
    Start-Sleep -Seconds $DurationSeconds
}
finally {
    if (-not $logProcess.HasExited) {
        $logProcess.Kill()
        $logProcess.WaitForExit()
    }
}

Write-Host "Capture complete: $OutputPath"
