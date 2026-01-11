cmake_minimum_required(VERSION 3.16)

# Force 32-bit build for MSVC to match available libraries
if (MSVC)
  # For Ninja generator, we need to configure the toolchain before project()
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project requires 32-bit libraries. Please configure CMake with: -DCMAKE_GENERATOR_PLATFORM=Win32 (for Visual Studio generator) or use the x86 MSVC toolchain in CLion settings")
  endif()
endif()

project(MuMain)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Build type (default RelWithDebInfo for useful symbols)" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Collect all C++ source files under source/
file(GLOB_RECURSE MU_MAIN_SOURCES
  CONFIGURE_DEPENDS
  "source/*.cpp"
)

add_executable(Main ${MU_MAIN_SOURCES})

if (NOT MSVC)
  set(MU_MAIN_MAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/Main.map")
  target_link_options(Main PRIVATE "-Wl,-Map,${MU_MAIN_MAP_FILE}")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Build as a Windows GUI application (no console window) and let CMake
  # add the default system libraries like kernel32, user32, gdi32, etc.
  set_target_properties(Main PROPERTIES
    WIN32_EXECUTABLE TRUE
  )

  enable_language(RC)
  target_sources(Main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/source/resource.rc"
  )
endif()

# Skeleton Linux-native linkage. This intentionally depends only on
# ubiquitous system libraries so we can focus on getting the project to
# compile; more platform-specific adjustments will follow as we iterate.
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(Main PRIVATE
    pthread
    dl
    m
  )
endif()

target_include_directories(Main PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/source"
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/netcore/includes"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends"
)

# Basic compile definitions, roughly mirroring the original MSVC project
# (fine-tuning per configuration can be done later if needed.)
target_compile_definitions(Main PRIVATE
  _LANGUAGE_FOREIGN
  _LANGUAGE_ENG
  UNICODE
  _USE_32BIT_TIME_T  # Use 32-bit time_t for 32-bit builds
  $<$<CONFIG:Debug>:_DEBUG;_FOREIGN_DEBUG>
  $<$<CONFIG:Release>:NDEBUG;_FOREIGN_NDEBUG;LDS_PATCH_GLOBAL_100520>
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(Main PRIVATE
    STRSAFE_NO_DEPRECATE
  )
endif()

if (MSVC)
  target_compile_definitions(Main PRIVATE
    WIN32
    _WINDOWS
  )

  # Link against Windows system libraries and local static libs
  target_link_directories(Main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"
  )

  target_link_libraries(Main PRIVATE
    imm32
    vfw32
    dsound
    dxguid
    opengl32
    glu32
    winmm
    ws2_32
    version
    shlwapi
    crypt32
    wininet
    urlmon
    turbojpeg-static
    wzAudio
  )

  # Copy required DLLs to build directory for MSVC builds
  add_custom_command(TARGET Main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/bin/wzAudio.dll"
      "${CMAKE_CURRENT_SOURCE_DIR}/bin/glew32.dll"
      "${CMAKE_CURRENT_SOURCE_DIR}/bin/ogg.dll"
      "${CMAKE_CURRENT_SOURCE_DIR}/bin/vorbisfile.dll"
      $<TARGET_FILE_DIR:Main>
    COMMENT "Copying required DLLs to build directory"
  )
endif()

# MinGW (32-bit or 64-bit) cross-toolchains: link against the Windows/OpenGL
# system libraries so that gl*, wgl*, Imm*, timeGetTime, etc. resolve.
# For turbojpeg and wzAudio we link against the logically named libraries,
# which must be provided as MinGW-compatible .a import libraries.
if (CMAKE_CXX_COMPILER MATCHES "mingw32" OR CMAKE_CXX_COMPILER MATCHES "w64-mingw32")
  target_link_directories(Main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"
  )

  set(MU_TURBOJPEG_STATIC_LIB "" CACHE FILEPATH "Path to MinGW-w64 static libturbojpeg.a")
  if (MU_TURBOJPEG_STATIC_LIB STREQUAL "" OR MU_TURBOJPEG_STATIC_LIB MATCHES "-NOTFOUND$")
    unset(MU_TURBOJPEG_STATIC_LIB CACHE)
  endif()
  if (NOT MU_TURBOJPEG_STATIC_LIB)
    find_library(MU_TURBOJPEG_STATIC_LIB
      NAMES turbojpeg
      PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"
      NO_CMAKE_FIND_ROOT_PATH
    )
  endif()
  if (NOT MU_TURBOJPEG_STATIC_LIB)
    set(_MU_OLD_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES}")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(_MU_TURBOJPEG_SEARCH_PATHS "")
    foreach(_mu_root IN LISTS CMAKE_FIND_ROOT_PATH)
      list(APPEND _MU_TURBOJPEG_SEARCH_PATHS "${_mu_root}/lib" "${_mu_root}/usr/lib")
    endforeach()
    list(APPEND _MU_TURBOJPEG_SEARCH_PATHS "/usr/i686-w64-mingw32/lib" "/usr/x86_64-w64-mingw32/lib")
    find_library(MU_TURBOJPEG_STATIC_LIB
      NAMES turbojpeg
      PATHS ${_MU_TURBOJPEG_SEARCH_PATHS}
      NO_CMAKE_FIND_ROOT_PATH
    )
    unset(_MU_TURBOJPEG_SEARCH_PATHS)
    unset(_mu_root)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${_MU_OLD_FIND_LIBRARY_SUFFIXES}")
    unset(_MU_OLD_FIND_LIBRARY_SUFFIXES)
  endif()
  if (NOT MU_TURBOJPEG_STATIC_LIB OR MU_TURBOJPEG_STATIC_LIB MATCHES "\\.dll\\.a$")
    message(FATAL_ERROR "Static libturbojpeg not found (libturbojpeg.a) or CMake picked an import library (.dll.a): '${MU_TURBOJPEG_STATIC_LIB}'. Install/build a MinGW-w64 static libjpeg-turbo to avoid shipping libturbojpeg.dll, or set MU_TURBOJPEG_STATIC_LIB to the full path of libturbojpeg.a.")
  endif()

  target_link_libraries(Main PRIVATE
    imm32
    vfw32
    dsound
    dxguid
    opengl32
    glu32
    winmm
    ws2_32
    version
    shlwapi
    crypt32
    wininet
    urlmon
    gdi32
    user32
    ole32
    oleaut32
    uuid
    comdlg32
    advapi32
    ${MU_TURBOJPEG_STATIC_LIB}
    wzAudio
  )
endif()
