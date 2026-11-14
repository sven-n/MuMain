# macOS, with Homebrew's packages.
# What a target file sets is described in docs/build/README.md (Targets).

set(MU_PLATFORM_SOURCES "posix")
set(MU_TARGET_ADAPTERS
  "Core/Platform/Adapters/OpenSslCredentials.cpp"
  "Core/Platform/Adapters/CredentialHostPosix.cpp"
  "GameShop/FileDownloader/Adapters/CurlShopDownload.cpp"
  "Core/Utilities/Adapters/CpuUsagePosix.cpp"
  "Render/Renderer/Adapters/GpuDeviceSdlChoice.cpp")
set(MU_TARGET_DEPENDENCIES OpenSSL CURL Cryptopp TurboJPEG)
set(MU_TARGET_SYSTEM_LIBRARIES
  "-framework AudioToolbox" "-framework AudioUnit" "-framework CoreAudio" "-framework CoreFoundation")

# The network library loads from beside the executable.
set(MU_MANAGED_LIBRARY_FILE "MUnique.Client.Library.dylib")
set(MU_MANAGED_LIBRARY_BY_NAME 0)
if(CMAKE_OSX_ARCHITECTURES MATCHES "(^|;)arm64($|;)" OR CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
  set(MU_DOTNET_RID "osx-arm64")
  set(MU_DOTNET_PLATFORM "arm64")
else()
  set(MU_DOTNET_RID "osx-x64")
  set(MU_DOTNET_PLATFORM "x64")
endif()
# See linux.cmake.
set(MU_DOTNET_EXTRA_ARGS "-p:ci=true")
set(MU_DOTNET_USES_LIBRARY_PATH TRUE)
# Native AOT's macOS runtime links Brotli.
set(MU_DOTNET_PKG_MODULES libbrotlienc libbrotlidec libbrotlicommon)
