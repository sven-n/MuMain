# One provider per dependency a target file lists in MU_TARGET_DEPENDENCIES (cmake/targets/<target>.cmake).
#
# A provider defines MuDep::<Name> for the game library to link, or stops the configure naming the package, the target
# and what needs it. When MuDep::<Name> already exists, the provider is skipped: a target file that builds a dependency
# itself (a port that compiles Crypto++ from source, say) defines the target first, as a GLOBAL imported or an
# INTERFACE library.
#
# Providers are macros, not functions, so what find_package sets (OPENSSL_CRYPTO_LIBRARY, imported targets) stays
# visible to src/CMakeLists.txt.
include_guard(GLOBAL)

macro(mu_dependency_not_found name need hint)
  message(FATAL_ERROR "${name} is needed by the ${MU_TARGET} target for ${need}, and was not found. ${hint}")
endmacro()

# A GLOBAL imported target, so the tests' directories resolve it too.
macro(mu_dependency_target name)
  add_library(MuDep_${name} INTERFACE)
  add_library(MuDep::${name} ALIAS MuDep_${name})
  foreach(_mu_item IN ITEMS ${ARGN})
    if(TARGET ${_mu_item})
      get_target_property(_mu_real ${_mu_item} ALIASED_TARGET)
      if(NOT _mu_real)
        set(_mu_real ${_mu_item})
      endif()
      get_target_property(_mu_imported ${_mu_real} IMPORTED)
      get_target_property(_mu_global ${_mu_real} IMPORTED_GLOBAL)
      if(_mu_imported AND NOT _mu_global)
        set_target_properties(${_mu_real} PROPERTIES IMPORTED_GLOBAL TRUE)
      endif()
    endif()
    target_link_libraries(MuDep_${name} INTERFACE ${_mu_item})
  endforeach()
endmacro()

# Saved logins (Core/Platform/Adapters/OpenSslCredentials.cpp).
macro(mu_provide_OpenSSL)
  find_package(OpenSSL COMPONENTS Crypto)
  if(NOT TARGET OpenSSL::Crypto)
    mu_dependency_not_found(OpenSSL "saved logins (OpenSslCredentials.cpp)"
      "Install OpenSSL's libcrypto (vcpkg's openssl, libssl-dev, Homebrew's openssl@3) or set OPENSSL_ROOT_DIR.")
  endif()
  mu_dependency_target(OpenSSL OpenSSL::Crypto)
endmacro()

# Shop downloads (GameShop/FileDownloader/Adapters/CurlShopDownload.cpp).
macro(mu_provide_CURL)
  find_package(CURL)
  if(NOT TARGET CURL::libcurl)
    mu_dependency_not_found(libcurl "shop downloads (CurlShopDownload.cpp)"
      "Install libcurl (vcpkg's curl, libcurl4-openssl-dev, Homebrew's curl) or set CURL_ROOT.")
  endif()
  mu_dependency_target(CURL CURL::libcurl)
endmacro()

# The ciphers of version 14 BMD models (Render/Models/BmdModulusCipher), linked statically so Main needs no Crypto++
# library at run time (the release builds ship none). vcpkg ships a CMake package with a static library; Debian/Ubuntu
# (libcrypto++-dev) and Homebrew (cryptopp) ship pkg-config files and install the static archive next to the shared
# one, which pkg-config would pick.
macro(mu_provide_Cryptopp)
  find_package(cryptopp CONFIG QUIET)
  if(TARGET cryptopp::cryptopp)
    mu_dependency_target(Cryptopp cryptopp::cryptopp)
  elseif(MSVC)
    mu_dependency_not_found(Crypto++ "version 14 BMD models" "Install vcpkg's cryptopp (vcpkg.json lists it).")
  else()
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
      pkg_search_module(CRYPTOPP libcrypto++ libcryptopp)
    endif()
    find_library(MU_CRYPTOPP_STATIC_LIBRARY NAMES libcrypto++.a libcryptopp.a HINTS ${CRYPTOPP_LIBRARY_DIRS})
    if(NOT CRYPTOPP_FOUND OR NOT MU_CRYPTOPP_STATIC_LIBRARY)
      mu_dependency_not_found(Crypto++ "version 14 BMD models"
        "Install libcrypto++-dev or Homebrew's cryptopp; the static library (libcrypto++.a or libcryptopp.a) is "
        "needed, found by pkg-config.")
    endif()
    mu_dependency_target(Cryptopp "${MU_CRYPTOPP_STATIC_LIBRARY}")
    target_include_directories(MuDep_Cryptopp INTERFACE ${CRYPTOPP_INCLUDE_DIRS})
  endif()
endmacro()

# JPEG textures (Render/Sprites/GlobalBitmapLoad.cpp).
macro(mu_provide_TurboJPEG)
  if(MSVC)
    # The checked-in static library, one per architecture.
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(_mu_turbojpeg "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib/x64/turbojpeg-static.lib")
    else()
      set(_mu_turbojpeg "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib/x86/turbojpeg-static.lib")
    endif()
    if(NOT EXISTS "${_mu_turbojpeg}")
      mu_dependency_not_found(libjpeg-turbo "JPEG textures" "'${_mu_turbojpeg}' is missing from the checkout.")
    endif()
    mu_dependency_target(TurboJPEG "${_mu_turbojpeg}")
    # It was built against the static CRT and asks for libcmt.lib, while the game and vcpkg's libraries use the DLL
    # CRT. Without libcmt, its objects use the DLL CRT too, and the linker no longer warns about two runtimes (LNK4098).
    target_link_options(MuDep_TurboJPEG INTERFACE /NODEFAULTLIB:libcmt.lib)
  elseif(CMAKE_CXX_COMPILER MATCHES "mingw32")
    # A MinGW-w64 static libturbojpeg.a, so no libturbojpeg.dll ships.
    set(MU_TURBOJPEG_STATIC_LIB "" CACHE FILEPATH "Path to MinGW-w64 static libturbojpeg.a")
    if(MU_TURBOJPEG_STATIC_LIB STREQUAL "" OR MU_TURBOJPEG_STATIC_LIB MATCHES "-NOTFOUND$")
      unset(MU_TURBOJPEG_STATIC_LIB CACHE)
    endif()
    if(NOT MU_TURBOJPEG_STATIC_LIB)
      find_library(MU_TURBOJPEG_STATIC_LIB NAMES turbojpeg PATHS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"
        NO_CMAKE_FIND_ROOT_PATH)
    endif()
    if(NOT MU_TURBOJPEG_STATIC_LIB)
      set(_mu_old_suffixes "${CMAKE_FIND_LIBRARY_SUFFIXES}")
      set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
      set(_mu_search_paths "")
      foreach(_mu_root IN LISTS CMAKE_FIND_ROOT_PATH)
        list(APPEND _mu_search_paths "${_mu_root}/lib" "${_mu_root}/usr/lib")
      endforeach()
      list(APPEND _mu_search_paths "/usr/i686-w64-mingw32/lib" "/usr/x86_64-w64-mingw32/lib")
      find_library(MU_TURBOJPEG_STATIC_LIB NAMES turbojpeg PATHS ${_mu_search_paths} NO_CMAKE_FIND_ROOT_PATH)
      set(CMAKE_FIND_LIBRARY_SUFFIXES "${_mu_old_suffixes}")
    endif()
    if(NOT MU_TURBOJPEG_STATIC_LIB OR MU_TURBOJPEG_STATIC_LIB MATCHES "\\.dll\\.a$")
      mu_dependency_not_found(libjpeg-turbo "JPEG textures"
        "Found '${MU_TURBOJPEG_STATIC_LIB}', not a static libturbojpeg.a. Install or build a MinGW-w64 static "
        "libjpeg-turbo, or set MU_TURBOJPEG_STATIC_LIB to the full path of libturbojpeg.a.")
    endif()
    mu_dependency_target(TurboJPEG "${MU_TURBOJPEG_STATIC_LIB}")
  else()
    find_library(TURBOJPEG_LIBRARY NAMES turbojpeg PATHS "/opt/homebrew/lib" "/usr/local/lib")
    if(NOT TURBOJPEG_LIBRARY)
      mu_dependency_not_found(libjpeg-turbo "JPEG textures"
        "Install libturbojpeg0-dev or Homebrew's jpeg-turbo, or set TURBOJPEG_LIBRARY.")
    endif()
    mu_dependency_target(TurboJPEG "${TURBOJPEG_LIBRARY}")
  endif()
endmacro()

# Links MuDep::<Name> for each dependency the target lists, providing the ones the target file did not define.
macro(mu_link_target_dependencies library)
  foreach(_mu_dependency IN LISTS MU_TARGET_DEPENDENCIES)
    if(NOT TARGET MuDep::${_mu_dependency})
      if(NOT COMMAND mu_provide_${_mu_dependency})
        message(FATAL_ERROR "The ${MU_TARGET} target lists the dependency '${_mu_dependency}', which has no provider in "
          "cmake/MuDependencies.cmake: define MuDep::${_mu_dependency} in the target file, or add a provider.")
      endif()
      cmake_language(CALL mu_provide_${_mu_dependency})
    endif()
    target_link_libraries(${library} PUBLIC MuDep::${_mu_dependency})
  endforeach()
endmacro()
