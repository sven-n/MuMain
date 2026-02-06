cmake_minimum_required(VERSION 3.25)

project(MuMain)

# Initialize imgui submodule automatically if missing
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp")
  find_package(Git QUIET)
  if(NOT GIT_FOUND)
    message(FATAL_ERROR "Git not found. Please run 'git submodule update --init' manually from repository root.")
  endif()

  message(STATUS "Initializing imgui submodule...")

  # Get repository root (parent of src/)
  cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH REPO_ROOT)

  # If directory exists but is incomplete, remove it first
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui")
    message(STATUS "Removing incomplete imgui directory...")
    file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui")
  endif()

  # Check if submodule is registered in git index
  execute_process(
    COMMAND ${GIT_EXECUTABLE} ls-files -s src/ThirdParty/imgui
    WORKING_DIRECTORY ${REPO_ROOT}
    OUTPUT_VARIABLE SUBMODULE_IN_INDEX
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(SUBMODULE_IN_INDEX STREQUAL "")
    # Submodule not in index - need to add it
    message(STATUS "Submodule not in git index, adding it...")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule add --force https://github.com/ocornut/imgui.git src/ThirdParty/imgui
      WORKING_DIRECTORY ${REPO_ROOT}
      RESULT_VARIABLE GIT_ADD_RESULT
      ERROR_VARIABLE GIT_ADD_ERROR
      OUTPUT_VARIABLE GIT_ADD_OUTPUT
    )
    if(NOT GIT_ADD_RESULT EQUAL "0")
      message(FATAL_ERROR "Failed to add submodule:\n${GIT_ADD_ERROR}${GIT_ADD_OUTPUT}")
    endif()
  else()
    # Submodule in index - just update it
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule sync
      WORKING_DIRECTORY ${REPO_ROOT}
      OUTPUT_QUIET ERROR_QUIET
    )
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init
      WORKING_DIRECTORY ${REPO_ROOT}
      RESULT_VARIABLE GIT_RESULT
      ERROR_VARIABLE GIT_ERROR
      OUTPUT_VARIABLE GIT_OUTPUT
    )
    if(NOT GIT_RESULT EQUAL "0")
      message(FATAL_ERROR "Submodule initialization failed:\n${GIT_ERROR}${GIT_OUTPUT}")
    endif()
  endif()

  message(STATUS "ImGui submodule initialized successfully")

  # Verify it worked
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp")
    message(FATAL_ERROR "ImGui submodule initialization appeared to succeed but imgui.cpp is still missing.")
  endif()
endif()

# Determine architecture if CMAKE_SIZEOF_VOID_P isn't set
if (NOT DEFINED CMAKE_SIZEOF_VOID_P OR CMAKE_SIZEOF_VOID_P EQUAL 0)
  # Try to detect from compiler path
  if (CMAKE_CXX_COMPILER MATCHES "x64|amd64|AMD64")
    set(CMAKE_SIZEOF_VOID_P 8 CACHE INTERNAL "Pointer size" FORCE)
    message(STATUS "Detected x64 from compiler path, setting CMAKE_SIZEOF_VOID_P=8")
  elseif (CMAKE_CXX_COMPILER MATCHES "x86|i386|i686")
    set(CMAKE_SIZEOF_VOID_P 4 CACHE INTERNAL "Pointer size" FORCE)
    message(STATUS "Detected x86 from compiler path, setting CMAKE_SIZEOF_VOID_P=4")
  else()
    # Default fallback
    set(CMAKE_SIZEOF_VOID_P 4 CACHE INTERNAL "Pointer size" FORCE)
    message(WARNING "Could not detect architecture, defaulting to x86")
  endif()
endif()

# Check architecture and configure accordingly
if (MSVC)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Configuring for 64-bit (x64) build")
  else()
    message(STATUS "Configuring for 32-bit (x86) build")
  endif()
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Build type (default RelWithDebInfo for useful symbols)" FORCE)
endif()

# Editor mode option (for admin builds)
option(ENABLE_EDITOR "Enable editor features for admin builds" OFF)

# Collect all C++ source files under source/
file(GLOB_RECURSE MU_MAIN_SOURCES
  CONFIGURE_DEPENDS
        "source/*.cpp"
)

# Add MuEditor sources when editor is enabled
if(ENABLE_EDITOR)
  file(GLOB_RECURSE MU_EDITOR_SOURCES
    CONFIGURE_DEPENDS
    "MuEditor/*.cpp"
  )
  list(APPEND MU_MAIN_SOURCES ${MU_EDITOR_SOURCES})
endif()

# ImGui static library (required when editor is enabled)
if(ENABLE_EDITOR)
  add_library(imgui STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_tables.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_win32.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_opengl2.cpp"
  )

  target_include_directories(imgui PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends"
  )

  # <-- Force C++20 for ImGui here
  target_compile_features(imgui PUBLIC cxx_std_20)
endif()

add_executable(Main ${MU_MAIN_SOURCES})

# <-- Force C++20 for Main here
target_compile_features(Main PUBLIC cxx_std_20)

if (NOT MSVC)
  set(MU_MAIN_MAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/Main.map")
  target_link_options(Main PRIVATE "-Wl,-Map,${MU_MAIN_MAP_FILE}")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Build as a Windows GUI application (no console window) and let CMake
  # add the default system libraries like kernel32, user32, gdi32, etc.
  set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  message(STATUS "=== DEBUG: CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
  message(STATUS "=== DEBUG: CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")
  message(STATUS "=== DEBUG: CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
  message(STATUS "=== DEBUG: Assets directory: ${ASSETS_DIR}")
  message(STATUS "=== DEBUG: Executable will be at: $<TARGET_FILE:Main>")
  message(STATUS "=== DEBUG: Executable directory: $<TARGET_FILE_DIR:Main>")
  set_target_properties(Main PROPERTIES
    WIN32_EXECUTABLE TRUE
  )

  enable_language(RC)
  target_sources(Main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/source/resource.rc"
  )

  # Copy assets directory next to executable (portable, IDE-agnostic solution)
  add_custom_command(
    TARGET Main
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== DEBUG: Copying assets from ${ASSETS_DIR} to $<TARGET_FILE_DIR:Main>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSETS_DIR}"
            "$<TARGET_FILE_DIR:Main>"
    COMMENT "Copying game assets to executable directory"
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
  $<$<BOOL:${ENABLE_EDITOR}>:${CMAKE_CURRENT_SOURCE_DIR}/MuEditor>
)

# Basic compile definitions, roughly mirroring the original MSVC project
# (fine-tuning per configuration can be done later if needed.)
target_compile_definitions(Main PRIVATE
  _LANGUAGE_FOREIGN
  _LANGUAGE_ENG
  UNICODE
  $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},4>:_USE_32BIT_TIME_T>  # Use 32-bit time_t only for 32-bit builds
  $<$<CONFIG:Debug>:_DEBUG;_FOREIGN_DEBUG>
  $<$<CONFIG:Release>:NDEBUG;_FOREIGN_NDEBUG;LDS_PATCH_GLOBAL_100520>
  $<$<BOOL:${ENABLE_EDITOR}>:_EDITOR>  # Editor features for admin builds
)

if (MSVC)
  target_compile_definitions(Main PRIVATE
    WIN32
    _WINDOWS
  )

  # Link against Windows system libraries and local static libs
  # Use architecture-specific library directories
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(LIB_ARCH_DIR "x64")
  else()
    set(LIB_ARCH_DIR "x86")
  endif()

  target_link_directories(Main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib/${LIB_ARCH_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"  # Fallback for arch-independent libs
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

  # Locate required DLLs in bin directory
  set(DLL_SEARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set(REQUIRED_DLL_NAMES wzAudio glew32 ogg vorbisfile)
  set(REQUIRED_DLLS)

  foreach(DLL_NAME IN LISTS REQUIRED_DLL_NAMES)
    find_file(${DLL_NAME}_DLL
      NAMES "${DLL_NAME}.dll"
      PATHS "${DLL_SEARCH_PATH}"
      NO_DEFAULT_PATH
      REQUIRED
    )
    list(APPEND REQUIRED_DLLS "${${DLL_NAME}_DLL}")
  endforeach()

  # Copy required DLLs to build directory for MSVC builds
  add_custom_command(TARGET Main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== DEBUG: Copying DLLs to: $<TARGET_FILE_DIR:Main>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${REQUIRED_DLLS}
      $<TARGET_FILE_DIR:Main>
    COMMENT "Copying required DLLs to build directory"
  )
endif()

# .NET Client Library and tools (platform-agnostic, requires dotnet SDK)
# Native AOT can only target the OS it runs on, so when cross-compiling from
# WSL we need the Windows dotnet.exe (available via WSL interop) rather than
# a Linux dotnet.
find_program(DOTNET_EXECUTABLE dotnet.exe)
if (NOT DOTNET_EXECUTABLE)
  find_program(DOTNET_EXECUTABLE dotnet)
endif()
if (DOTNET_EXECUTABLE)
  # Native AOT cannot cross-compile across OS boundaries.  A Linux dotnet
  # targeting win-x86/win-x64 will fail.  Only proceed when we have a Windows
  # dotnet.exe (native or via WSL interop).
  if (NOT DOTNET_EXECUTABLE MATCHES "\\.exe$" AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(WARNING "Found Linux dotnet but target is Windows. "
      "Cross-OS Native AOT is not supported. MUnique.Client.Library.dll will NOT be built. "
      "Install the Windows .NET SDK or use WSL interop (dotnet.exe) to enable.")
    set(DOTNET_EXECUTABLE "")
  endif()
endif()
if (DOTNET_EXECUTABLE)
  message(STATUS "Found .NET SDK: ${DOTNET_EXECUTABLE}")

  # Helper macro: convert a path to Windows-native format when using
  # Windows dotnet.exe from WSL.  On all other platforms this is a no-op.
  macro(mu_native_path _input _output)
    set(${_output} "${_input}")
    if (DOTNET_EXECUTABLE MATCHES "\\.exe$" AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
      execute_process(
        COMMAND wslpath -w "${_input}"
        OUTPUT_VARIABLE _mu_native_result
        RESULT_VARIABLE _mu_native_rc
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      if (_mu_native_rc EQUAL 0)
        set(${_output} "${_mu_native_result}")
      else()
        message(FATAL_ERROR "wslpath failed for '${_input}'. Ensure wslpath is available.")
      endif()
    endif()
  endmacro()

  # 1. Define the output path using a variable CMake can understand early
  set(DOTNET_DLL_PATH "${CMAKE_CURRENT_BINARY_DIR}/MUnique.Client.Library.dll")
  set(DOTNET_PROJ "${CMAKE_CURRENT_SOURCE_DIR}/../ClientLibrary/MUnique.Client.Library.csproj")

  # Create temp directories for Native AOT build.
  set(DOTNET_TEMP_OUTPUT "${CMAKE_BINARY_DIR}/dotnet_out")
  set(DOTNET_TEMP_DIR "${CMAKE_BINARY_DIR}/.dotnet_temp")

  # NuGet cache: defaults to <project>/.nuget, override with -DMU_NUGET_CACHE_DIR=...
  set(MU_NUGET_CACHE_DIR "${CMAKE_SOURCE_DIR}/.nuget" CACHE PATH "NuGet package cache directory")
  set(DOTNET_NUGET_DIR "${MU_NUGET_CACHE_DIR}")
  file(MAKE_DIRECTORY "${DOTNET_TEMP_OUTPUT}")
  file(MAKE_DIRECTORY "${DOTNET_TEMP_DIR}")
  file(MAKE_DIRECTORY "${DOTNET_NUGET_DIR}")

  # Convert paths to Windows-native format for MSBuild (no-op outside WSL).
  mu_native_path("${DOTNET_PROJ}" DOTNET_PROJ_NATIVE)
  mu_native_path("${DOTNET_TEMP_OUTPUT}" DOTNET_TEMP_OUTPUT_NATIVE)
  mu_native_path("${DOTNET_NUGET_DIR}" DOTNET_NUGET_DIR_NATIVE)
  mu_native_path("${DOTNET_TEMP_DIR}" DOTNET_TEMP_DIR_NATIVE)

  # 2. Tell CMake to watch ALL .cs files in the ClientLibrary folder
  file(GLOB_RECURSE DOTNET_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../ClientLibrary/*.cs")

  # 3. Build to temp directory with Native AOT, then copy to build and bin directories
  # Override environment variables to avoid issues with umlauts in Windows username
  # Determine runtime identifier and platform target based on architecture
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DOTNET_RID "win-x64")
    set(DOTNET_PLATFORM "x64")
    message(STATUS ".NET Client Library will build for x64 (CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})")
  else()
    set(DOTNET_RID "win-x86")
    set(DOTNET_PLATFORM "x86")
    message(STATUS ".NET Client Library will build for x86 (CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})")
  endif()

  add_custom_command(
          OUTPUT "${DOTNET_DLL_PATH}"
          COMMAND ${CMAKE_COMMAND} -E echo "--- C# Changes Detected: Rebuilding Client Library ---"
          COMMAND ${CMAKE_COMMAND} -E env
                  "NUGET_PACKAGES=${DOTNET_NUGET_DIR_NATIVE}"
                  "DOTNET_CLI_HOME=${DOTNET_TEMP_DIR_NATIVE}"
                  "TEMP=${DOTNET_TEMP_DIR_NATIVE}"
                  "TMP=${DOTNET_TEMP_DIR_NATIVE}"
                  "${DOTNET_EXECUTABLE}" publish "${DOTNET_PROJ_NATIVE}" -c $<CONFIG> -r ${DOTNET_RID} -p:PlatformTarget=${DOTNET_PLATFORM} -o "${DOTNET_TEMP_OUTPUT_NATIVE}" --nologo
          COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DOTNET_TEMP_OUTPUT}/MUnique.Client.Library.dll" "${DOTNET_DLL_PATH}"
          DEPENDS "${DOTNET_PROJ}" ${DOTNET_SOURCES}
          COMMENT "Checking for .NET Client Library updates..."
          VERBATIM
  )

  # 4. Create a target for the DLL and link it to Main
  add_custom_target(ClientLibrary DEPENDS "${DOTNET_DLL_PATH}")
  add_dependencies(Main ClientLibrary)
  set_target_properties(ClientLibrary PROPERTIES EXCLUDE_FROM_ALL TRUE)

  # Copy ClientLibrary DLL to the executable's output directory
  add_custom_command(TARGET Main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== DEBUG: Copying ClientLibrary DLL from: ${DOTNET_DLL_PATH}"
    COMMAND ${CMAKE_COMMAND} -E echo "=== DEBUG: Copying ClientLibrary DLL to: $<TARGET_FILE_DIR:Main>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${DOTNET_DLL_PATH}"
      $<TARGET_FILE_DIR:Main>
    COMMAND ${CMAKE_COMMAND} -E echo "=== DEBUG: ClientLibrary DLL copy complete"
    COMMENT "Copying ClientLibrary DLL to build directory"
  )

  # ConstantsReplacer build target
  set(CONSTANTS_REPLACER_PROJ "${CMAKE_CURRENT_SOURCE_DIR}/../ConstantsReplacer/ConstantsReplacer.csproj")
  mu_native_path("${CONSTANTS_REPLACER_PROJ}" CONSTANTS_REPLACER_PROJ_NATIVE)
  mu_native_path("${CMAKE_BINARY_DIR}/ConstantsReplacer" CONSTANTS_REPLACER_OUTDIR_NATIVE)
  file(GLOB_RECURSE CONSTANTS_REPLACER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../ConstantsReplacer/*.cs")
  set(CONSTANTS_REPLACER_OUTPUT "${CMAKE_BINARY_DIR}/ConstantsReplacer/ConstantsReplacer.exe")

  add_custom_command(
    OUTPUT "${CONSTANTS_REPLACER_OUTPUT}"
    COMMAND "${DOTNET_EXECUTABLE}" build "${CONSTANTS_REPLACER_PROJ_NATIVE}" -c $<CONFIG> -o "${CONSTANTS_REPLACER_OUTDIR_NATIVE}" --nologo
    DEPENDS "${CONSTANTS_REPLACER_PROJ}" ${CONSTANTS_REPLACER_SOURCES}
    COMMENT "Building ConstantsReplacer tool..."
    VERBATIM
  )

  add_custom_target(ConstantsReplacer DEPENDS "${CONSTANTS_REPLACER_OUTPUT}")
else()
  message(WARNING ".NET SDK not found. MUnique.Client.Library.dll will NOT be built. "
    "The game will run but cannot connect to the server. Install .NET SDK 10.0+ to enable.")
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

# Link ImGui for all platforms when editor is enabled
if(ENABLE_EDITOR)
  target_link_libraries(Main PRIVATE imgui)
  # MinGW's linker is order-sensitive: opengl32 and dwmapi (needed by ImGui
  # backends) must appear after the imgui static library.  MSVC already links
  # these earlier and doesn't care about order.
  if (NOT MSVC)
    target_link_libraries(Main PRIVATE opengl32 dwmapi)
  endif()
endif()
