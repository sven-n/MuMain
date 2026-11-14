# Linux, with the distribution's packages.
# What a target file sets is described in docs/build/README.md (Targets).

set(MU_PLATFORM_SOURCES "posix")
set(MU_TARGET_ADAPTERS
  "Core/Platform/Adapters/OpenSslCredentials.cpp"
  "Core/Platform/Adapters/CredentialHostPosix.cpp"
  "GameShop/FileDownloader/Adapters/CurlShopDownload.cpp"
  "Core/Utilities/Adapters/CpuUsagePosix.cpp"
  "Render/Renderer/Adapters/GpuDeviceSdlChoice.cpp")
set(MU_TARGET_DEPENDENCIES OpenSSL CURL Cryptopp TurboJPEG)
set(MU_TARGET_SYSTEM_LIBRARIES pthread dl m)

# The network library loads from beside the executable.
set(MU_MANAGED_LIBRARY_FILE "MUnique.Client.Library.so")
set(MU_MANAGED_LIBRARY_BY_NAME 0)
set(MU_DOTNET_RID "linux-x64")
set(MU_DOTNET_PLATFORM "x64")
# The PreBuild XSLT step regenerates files with Windows-style backslash paths and corrupts them here; the generated
# sources are committed, so skip it (ci=true).
set(MU_DOTNET_EXTRA_ARGS "-p:ci=true")
# Native AOT links with the system linker, which searches LIBRARY_PATH.
set(MU_DOTNET_USES_LIBRARY_PATH TRUE)
