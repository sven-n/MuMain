param(
    [string]$NativeLibraryDirectory,
    [string]$OutputPath
)

$ErrorActionPreference = 'Stop'

$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))

if (-not $NativeLibraryDirectory) {
    $NativeLibraryDirectory = Join-Path $repositoryRoot 'out\android\native'
}
$NativeLibraryDirectory = [IO.Path]::GetFullPath($NativeLibraryDirectory)

if (-not $OutputPath) {
    $OutputPath = Join-Path $repositoryRoot 'out\android\MuMain-debug.apk'
}
$OutputPath = [IO.Path]::GetFullPath($OutputPath)

$sourceGradleWrapper = Join-Path $repositoryRoot 'src\ThirdParty\SDL\android-project\gradlew.bat'
if (-not (Test-Path -LiteralPath $sourceGradleWrapper)) {
    throw 'SDL Gradle wrapper is missing. Run git submodule update --init --recursive.'
}
if (-not (Test-Path -LiteralPath $NativeLibraryDirectory)) {
    throw "Native library directory does not exist: $NativeLibraryDirectory"
}

if (-not $env:JAVA_HOME) {
    $androidStudioJbr = 'C:\Program Files\Android\Android Studio\jbr'
    if (Test-Path -LiteralPath $androidStudioJbr) {
        $env:JAVA_HOME = $androidStudioJbr
    }
}
if (-not $env:ANDROID_HOME -and -not $env:ANDROID_SDK_ROOT) {
    $defaultAndroidSdk = Join-Path $env:LOCALAPPDATA 'Android\Sdk'
    if (Test-Path -LiteralPath $defaultAndroidSdk) {
        $env:ANDROID_HOME = $defaultAndroidSdk
    }
}

$buildRoot = $repositoryRoot
$buildNativeLibraryDirectory = $NativeLibraryDirectory
$stagingRoot = $null

try {
    if ($repositoryRoot.StartsWith('\\')) {
        $stagingRoot = Join-Path ([IO.Path]::GetTempPath()) `
            ("mumain-android-package-{0}" -f [guid]::NewGuid().ToString('N'))
        New-Item -ItemType Directory -Path $stagingRoot | Out-Null

        Copy-Item -LiteralPath (Join-Path $repositoryRoot 'android') `
            -Destination $stagingRoot -Recurse

        $stagedSdlProject = Join-Path $stagingRoot 'src\ThirdParty\SDL\android-project'
        New-Item -ItemType Directory -Path $stagedSdlProject -Force | Out-Null
        Copy-Item -LiteralPath $sourceGradleWrapper -Destination $stagedSdlProject
        Copy-Item -LiteralPath (Join-Path $repositoryRoot `
                'src\ThirdParty\SDL\android-project\gradle') `
            -Destination $stagedSdlProject -Recurse

        $stagedSdlMain = Join-Path $stagedSdlProject 'app\src\main'
        New-Item -ItemType Directory -Path $stagedSdlMain -Force | Out-Null
        Copy-Item -LiteralPath (Join-Path $repositoryRoot `
                'src\ThirdParty\SDL\android-project\app\src\main\java') `
            -Destination $stagedSdlMain -Recurse

        $stagedNativeParent = Join-Path $stagingRoot 'out\android'
        New-Item -ItemType Directory -Path $stagedNativeParent -Force | Out-Null
        Copy-Item -LiteralPath $NativeLibraryDirectory `
            -Destination $stagedNativeParent -Recurse

        $buildRoot = $stagingRoot
        $buildNativeLibraryDirectory = Join-Path $stagedNativeParent 'native'
    }

    $androidProject = Join-Path $buildRoot 'android'
    $gradleWrapper = Join-Path $buildRoot `
        'src\ThirdParty\SDL\android-project\gradlew.bat'

    & $gradleWrapper `
        --project-dir $androidProject `
        "-PmuNativeLibDir=$buildNativeLibraryDirectory" `
        :app:assembleDebug
    if ($LASTEXITCODE -ne 0) {
        throw "Gradle APK build failed with exit code $LASTEXITCODE."
    }

    $builtApk = Join-Path $androidProject `
        'app\build\outputs\apk\debug\app-debug.apk'
    if (-not (Test-Path -LiteralPath $builtApk)) {
        throw "Gradle completed without producing $builtApk"
    }

    $outputDirectory = Split-Path -Parent $OutputPath
    New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
    Copy-Item -LiteralPath $builtApk -Destination $OutputPath -Force
}
finally {
    if ($stagingRoot -and (Test-Path -LiteralPath $stagingRoot)) {
        $resolvedStaging = [IO.Path]::GetFullPath($stagingRoot)
        $resolvedTemp = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
        if ($resolvedStaging.StartsWith($resolvedTemp) -and
            (Split-Path -Leaf $resolvedStaging).StartsWith('mumain-android-package-')) {
            Remove-Item -LiteralPath $resolvedStaging -Recurse -Force
        }
    }
}

$apk = Get-Item -LiteralPath $OutputPath
$hash = Get-FileHash -LiteralPath $OutputPath -Algorithm SHA256
Write-Host "APK ready: $($apk.FullName)"
Write-Host "Size: $($apk.Length) bytes"
Write-Host "SHA-256: $($hash.Hash)"
