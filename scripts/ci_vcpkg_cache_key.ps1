$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($env:ImageVersion)) {
    throw 'ImageVersion is required to identify the hosted runner toolchain.'
}
$compiler = (Get-Command cl.exe -ErrorAction Stop).Source
$compilerHash = (Get-FileHash -LiteralPath $compiler -Algorithm SHA256).Hash
$vcpkgRevision = git -C C:/vcpkg rev-parse HEAD
if ($LASTEXITCODE -ne 0) {
    throw 'Could not identify the vcpkg tool revision.'
}
"identity=$($env:ImageVersion)-$compilerHash-$vcpkgRevision" |
    Out-File -FilePath $env:GITHUB_OUTPUT -Append -Encoding utf8
# The compiler cache's name, which only the compiler picks, in the form the Linux and macOS jobs use.
"compiler=$($compilerHash.Substring(0, 16).ToLowerInvariant())" |
    Out-File -FilePath $env:GITHUB_OUTPUT -Append -Encoding utf8
