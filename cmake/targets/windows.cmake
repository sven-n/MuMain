# Windows: MSVC with vcpkg's packages, or a MinGW-w64 cross toolchain.
# What a target file sets is described in docs/build/README.md (Targets).

set(MU_PLATFORM_SOURCES "win32")
set(MU_TARGET_ADAPTERS
  "Core/Platform/Adapters/OpenSslCredentials.cpp"
  "Core/Platform/Adapters/CredentialHostWin32.cpp"
  "GameShop/FileDownloader/Adapters/CurlShopDownload.cpp"
  "Core/Utilities/Adapters/CpuUsageWin32.cpp"
  "Render/Renderer/Adapters/GpuDevicePreferVulkan.cpp")
set(MU_TARGET_DEPENDENCIES OpenSSL CURL Cryptopp TurboJPEG)
set(MU_TARGET_SYSTEM_LIBRARIES imm32 vfw32 dxguid winmm ws2_32 version shlwapi crypt32)
if(CMAKE_CXX_COMPILER MATCHES "mingw32")
  list(APPEND MU_TARGET_SYSTEM_LIBRARIES gdi32 user32 ole32 oleaut32 uuid comdlg32 advapi32)
endif()

# The network library loads by bare name from the executable's folder.
set(MU_MANAGED_LIBRARY_FILE "MUnique.Client.Library.dll")
set(MU_MANAGED_LIBRARY_BY_NAME 1)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(MU_DOTNET_RID "win-x64")
  set(MU_DOTNET_PLATFORM "x64")
else()
  set(MU_DOTNET_RID "win-x86")
  set(MU_DOTNET_PLATFORM "x86")
endif()
set(MU_DOTNET_EXTRA_ARGS "")
