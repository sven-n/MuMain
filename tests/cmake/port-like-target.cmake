# A target file shaped like a port's, for checking that the build is ready for one (docs/build/README.md, Targets):
# the host's target file, then the choices a port makes. It compiles the "unavailable" adapters, so it needs neither
# OpenSSL nor libcurl; it brings its own network library adapter, which has no library; and it builds SDL and the game
# as shared libraries, as Android loads them.
#
# Configure with -DMU_TARGET_FILE=<this file> -DBUILD_TESTING=OFF, and CMAKE_DISABLE_FIND_PACKAGE_OpenSSL=ON and
# CMAKE_DISABLE_FIND_PACKAGE_CURL=ON to prove neither is looked for. MU_PORT_LIKE_EXECUTABLE=ON keeps SDL static, the
# game an executable and the NativeAOT library, so the result runs and connects on the host.

include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/targets/${MU_TARGET}.cmake")

set(MU_TARGET_ADAPTERS
  "Core/Platform/Adapters/CredentialsUnavailable.cpp"
  "GameShop/FileDownloader/Adapters/ShopDownloadUnavailable.cpp"
  "Core/Utilities/Adapters/CpuUsageUnavailable.cpp"
  "Render/Renderer/Adapters/GpuDeviceSdlChoice.cpp")
set(MU_TARGET_DEPENDENCIES Cryptopp TurboJPEG)

if(NOT MU_PORT_LIKE_EXECUTABLE)
  set(MU_SDL_SHARED ON)
  set(MU_APP_LIBRARY ON)
  set(MU_APP_OUTPUT_NAME "main")
  # Its own network library adapter, which also skips building the NativeAOT library.
  set(MU_MANAGED_SYMBOL_SOURCE "port-like")
  set(MU_TARGET_SYMBOL_ADAPTER "${CMAKE_CURRENT_LIST_DIR}/PortLikeSymbolSource.cpp")
endif()
