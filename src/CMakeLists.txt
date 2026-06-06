cmake_minimum_required(VERSION 3.25)

project(MuMain)

# Repository root (parent of this src/ directory). Defined here so all
# sibling-directory references (cmake/, ClientLibrary/, etc.) have a
# single, stable anchor instead of scattered "../" navigations.
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH REPO_ROOT)

include("${REPO_ROOT}/cmake/EnsureSubmodule.cmake")

mu_ensure_submodule("ThirdParty/SDL"
  INDICATOR "CMakeLists.txt"
  URL "https://github.com/libsdl-org/SDL.git")

mu_ensure_submodule("ThirdParty/SDL_mixer"
  INDICATOR "CMakeLists.txt"
  URL "https://github.com/libsdl-org/SDL_mixer.git")

# SDL3 — static link, no tests, no install
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/SDL EXCLUDE_FROM_ALL)

# SDL3_mixer — static link, MP3 + Ogg Vorbis only (matches our music assets)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDLMIXER_INSTALL OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VENDORED ON CACHE BOOL "" FORCE)
set(SDLMIXER_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MP3 ON CACHE BOOL "" FORCE)
set(SDLMIXER_MP3_DRMP3 ON CACHE BOOL "" FORCE)
# mpg123 is built as a dynamically loaded library — drmp3 alone is enough
# for the assets we ship and keeps everything statically linked.
set(SDLMIXER_MP3_MPG123 OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_STB ON CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_VORBISFILE OFF CACHE BOOL "" FORCE)
set(SDLMIXER_FLAC OFF CACHE BOOL "" FORCE)
set(SDLMIXER_OPUS OFF CACHE BOOL "" FORCE)
set(SDLMIXER_WAVPACK OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MOD OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MIDI OFF CACHE BOOL "" FORCE)
set(SDLMIXER_GME OFF CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/SDL_mixer EXCLUDE_FROM_ALL)

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

# ---------------------------------------------------------------------------
# Engine/game code -> static library (libMuClient); MuMain -> thin executable.
#
# All engine and game logic compiles into the MuClient static library. The
# Main executable defined further down contains only the platform entry point
# (WinMain) and links the library. Keeping the two separated lets unit-test
# binaries link the same engine code instead of recompiling individual
# translation units, and stops bootstrap-only code from leaking into the
# engine (issue #345).
# ---------------------------------------------------------------------------

# Collect all C++ source files under source/
file(GLOB_RECURSE MU_CLIENT_SOURCES
  CONFIGURE_DEPENDS
        "source/*.cpp"
)

# Per-platform entry points (App/Platform/<OS>/Winmain.cpp, main.cpp, main.mm)
# belong to the executable, not the engine library. Removing them keeps the
# library free of a WinMain/main() so it can be linked into test binaries that
# bring their own entry point, and so a non-Windows entry (e.g. Linux main.cpp)
# does not get globbed into MuClient and collide with the executable's own
# entry symbol at link time. Match only the Platform/<OS>/ entry-file tail, not
# an absolute path: a ${CMAKE_CURRENT_SOURCE_DIR}-built path can differ from
# glob results in drive-letter casing or separators on Windows. This excludes
# only the entry files; other Platform/<OS>/ sources (e.g. Local.cpp) stay in
# the library.
list(FILTER MU_CLIENT_SOURCES EXCLUDE REGEX "[/\\]Platform[/\\][^/\\]+[/\\][Ww]in[Mm]ain\\.[Cc]pp$")
list(FILTER MU_CLIENT_SOURCES EXCLUDE REGEX "[/\\]Platform[/\\][^/\\]+[/\\]main\\.(cpp|mm)$")

# Add MuEditor sources when editor is enabled
if(ENABLE_EDITOR)
  file(GLOB_RECURSE MU_EDITOR_SOURCES
    CONFIGURE_DEPENDS
    "MuEditor/*.cpp"
  )
  list(APPEND MU_CLIENT_SOURCES ${MU_EDITOR_SOURCES})
endif()

# ImGui static library (required when editor is enabled)
if(ENABLE_EDITOR)
  mu_ensure_submodule("ThirdParty/imgui"
    INDICATOR "imgui.cpp"
    URL "https://github.com/ocornut/imgui.git")

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

  # The Win32 + OpenGL2 backends call straight into opengl32 (glOrtho,
  # glShadeModel, ...) and dwmapi. Declaring them as PUBLIC dependencies of
  # imgui lets CMake place these libraries *after* libimgui.a on the final
  # link line, which MinGW's order-sensitive linker requires. Restrict to
  # Windows non-MSVC (MinGW): these are Windows-only libraries, and MSVC links
  # them elsewhere and ignores order anyway.
  if (WIN32 AND NOT MSVC)
    target_link_libraries(imgui PUBLIC opengl32 dwmapi)
  endif()

  # <-- Force C++20 for ImGui here
  target_compile_features(imgui PUBLIC cxx_std_20)
endif()

# Engine/game code as a static library. Compile- and link-time usage
# requirements (C++20, include paths, definitions, third-party libraries) are
# attached PUBLIC below so both the Main executable and any test binary that
# links MuClient inherit them automatically.
add_library(MuClient STATIC ${MU_CLIENT_SOURCES})
target_compile_features(MuClient PUBLIC cxx_std_20)

# The engine relies on self-registering globals whose constructors get
# silently dropped if the linker only pulls archive members on demand, so
# every consumer must link MuClient with WHOLE_ARCHIVE. Rather than make each
# consumer remember that, MuClient::MuClient is an interface wrapper that
# carries the WHOLE_ARCHIVE requirement (and, transitively, all of MuClient's
# public compile/link properties). Executables and unit tests just link
# MuClient::MuClient; link the raw MuClient target directly only when an
# on-demand link is genuinely wanted.
add_library(MuClient_whole INTERFACE)
target_link_libraries(MuClient_whole INTERFACE $<LINK_LIBRARY:WHOLE_ARCHIVE,MuClient>)
add_library(MuClient::MuClient ALIAS MuClient_whole)

# Thin executable: platform bootstrap only, links the engine library. The
# executable shell's sources live under source/App/ (see issue #346). The entry
# point is selected per platform (issue #441): each OS keeps its bootstrap under
# source/App/Platform/<OS>/, so adding a platform is a new entry file plus a
# branch here, not a CMake rewrite. Only the Windows entry point exists today;
# the Linux/macOS sources are added when that work starts.
#
# Unsupported platforms fail loudly rather than silently selecting the Linux
# entry: the mobile targets need a different entry model and a renderer port
# (the engine uses fixed-function desktop OpenGL), so quietly compiling a
# desktop main() for them would be wrong. iOS is matched before the generic
# APPLE/macOS branch, and Android before the generic UNIX/Linux branch.
if (WIN32)
  set(MU_APP_ENTRY "source/App/Platform/Windows/Winmain.cpp")
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
  message(FATAL_ERROR "iOS is not a supported build target yet: it needs an "
    "OpenGL ES/Metal renderer port and a touch UI, and distribution is App "
    "Store restricted. When that work starts, add source/App/Platform/iOS/ "
    "and replace this branch.")
elseif (APPLE)
  set(MU_APP_ENTRY "source/App/Platform/macOS/main.mm")
elseif (ANDROID)
  message(FATAL_ERROR "Android is not a supported build target yet: it needs "
    "an OpenGL ES renderer port, and its entry point is android_main via "
    "native_app_glue built as a shared library, not an executable. When that "
    "work starts, add source/App/Platform/Android/ and replace this branch.")
elseif (UNIX)
  set(MU_APP_ENTRY "source/App/Platform/Linux/main.cpp")
else()
  message(FATAL_ERROR "No executable entry point defined for this platform "
    "(CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}). Add source/App/Platform/<OS>/ "
    "and a branch above.")
endif()

add_executable(Main
  "${CMAKE_CURRENT_SOURCE_DIR}/${MU_APP_ENTRY}"
)
target_compile_features(Main PUBLIC cxx_std_20)
target_link_libraries(Main PRIVATE MuClient::MuClient)

if (NOT MSVC)
  set(MU_MAIN_MAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/Main.map")
  target_link_options(Main PRIVATE "-Wl,-Map,${MU_MAIN_MAP_FILE}")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Build as a Windows GUI application (no console window) and let CMake
  # add the default system libraries like kernel32, user32, gdi32, etc.
  message(STATUS "=== DEBUG: CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
  message(STATUS "=== DEBUG: CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")
  message(STATUS "=== DEBUG: CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
  message(STATUS "=== DEBUG: Executable will be at: $<TARGET_FILE:Main>")
  message(STATUS "=== DEBUG: Executable directory: $<TARGET_FILE_DIR:Main>")
  set_target_properties(Main PROPERTIES
    WIN32_EXECUTABLE TRUE
  )

  enable_language(RC)
  target_sources(Main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/source/App/Platform/Windows/resource.rc"
  )
endif()

# Copy assets directory next to executable. Platform-agnostic: any build
# that produces the Main executable needs the assets alongside it.
# The custom command's stamp file depends on every file under
# ${ASSETS_DIR}, so the copy only runs when at least one asset is newer
# than the stamp -- unchanged builds skip the copy entirely, edited
# assets re-trigger it. CONFIGURE_DEPENDS makes CMake re-evaluate the
# glob at build time so newly-added asset files are picked up without a
# manual reconfigure.
set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
message(STATUS "=== DEBUG: Assets directory: ${ASSETS_DIR}")
file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS "${ASSETS_DIR}/*")
# $<CONFIG> in the stamp name keeps multi-config generators (Visual
# Studio, Xcode) honest: each configuration's output directory needs
# its own stamp, otherwise switching from Debug to Release would see
# the Debug stamp and skip the copy, leaving the Release dir empty.
set(ASSETS_STAMP "${CMAKE_CURRENT_BINARY_DIR}/.assets_copied_$<CONFIG>.stamp")
add_custom_command(
  OUTPUT "${ASSETS_STAMP}"
  COMMAND ${CMAKE_COMMAND} -E echo "=== Copying assets from ${ASSETS_DIR} to $<TARGET_FILE_DIR:Main>"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${ASSETS_DIR}"
          "$<TARGET_FILE_DIR:Main>"
  COMMAND ${CMAKE_COMMAND} -E touch "${ASSETS_STAMP}"
  DEPENDS ${ASSET_FILES}
  COMMENT "Copying game assets (sources changed)"
  VERBATIM
)
add_custom_target(CopyAssets DEPENDS "${ASSETS_STAMP}")
add_dependencies(Main CopyAssets)

# Seed config.ini from the template only when it doesn't already exist
# next to the executable. The bulk asset copy ships config.ini.template
# into the build dir on every build; this step turns it into config.ini
# the first time and then leaves the user's edited copy alone forever.
add_custom_command(TARGET Main POST_BUILD
  COMMAND "${CMAKE_COMMAND}"
    "-DINPUT=${ASSETS_DIR}/config.ini.template"
    "-DOUTPUT=$<TARGET_FILE_DIR:Main>/config.ini"
    "-P" "${REPO_ROOT}/cmake/SeedConfigIfMissing.cmake"
  VERBATIM
)

# Skeleton Linux-native linkage. This intentionally depends only on
# ubiquitous system libraries so we can focus on getting the project to
# compile; more platform-specific adjustments will follow as we iterate.
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(MuClient PUBLIC
    pthread
    dl
    m
  )
endif()

target_include_directories(MuClient PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/source"
  # source/App holds the moved PCH header (stdafx.h). Keeping App/ on the
  # include path lets the ~350 bare #include "stdafx.h" resolve unchanged
  # after the bootstrap files moved out of source/ (issue #346).
  "${CMAKE_CURRENT_SOURCE_DIR}/source/App"
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/netcore/includes"
  $<$<BOOL:${ENABLE_EDITOR}>:${CMAKE_CURRENT_SOURCE_DIR}/MuEditor>
)

# Basic compile definitions, roughly mirroring the original MSVC project
# (fine-tuning per configuration can be done later if needed.)
target_compile_definitions(MuClient PUBLIC
  _LANGUAGE_FOREIGN
  _LANGUAGE_ENG
  UNICODE
  $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},4>:_USE_32BIT_TIME_T>  # Use 32-bit time_t only for 32-bit builds
  $<$<CONFIG:Debug>:_DEBUG;_FOREIGN_DEBUG>
  $<$<CONFIG:Release>:NDEBUG;_FOREIGN_NDEBUG;LDS_PATCH_GLOBAL_100520>
  $<$<BOOL:${ENABLE_EDITOR}>:_EDITOR>  # Editor features for admin builds
)

if (MSVC)
  target_compile_definitions(MuClient PUBLIC
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

  target_link_directories(MuClient PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib/${LIB_ARCH_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"  # Fallback for arch-independent libs
  )

  target_link_libraries(MuClient PUBLIC
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
  )

  # Locate required DLLs in bin directory
  set(DLL_SEARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set(REQUIRED_DLL_NAMES glew32 ogg vorbisfile)
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
  # Native AOT can't cross-compile across OS boundaries: a Linux dotnet
  # targeting win-x86/win-x64 will fail at publish time. Build-time C#
  # tools that emit C++ source (ConstantsReplacer, ResxGen) are unaffected
  # and only need a working dotnet on the host - so gate the ClientLibrary
  # step separately rather than clearing DOTNET_EXECUTABLE altogether.
  set(DOTNET_CAN_BUILD_CLIENT_LIBRARY TRUE)
  if (NOT DOTNET_EXECUTABLE MATCHES "\\.exe$" AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(WARNING "Found Linux dotnet but target is Windows. "
      "Cross-OS Native AOT is not supported. MUnique.Client.Library.dll will NOT be built. "
      "Install the Windows .NET SDK or use WSL interop (dotnet.exe) to enable.")
    set(DOTNET_CAN_BUILD_CLIENT_LIBRARY FALSE)
  endif()
endif()
if (DOTNET_EXECUTABLE)
  message(STATUS "Found .NET SDK: ${DOTNET_EXECUTABLE}")

  # Helper macro: convert a path to Windows-native format when using
  # Windows dotnet.exe from WSL.  On all other platforms this is a no-op.
  function(mu_native_path input_path output_var)
    if (DOTNET_EXECUTABLE MATCHES "\\.exe$" AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
      execute_process(
        COMMAND wslpath -w "${input_path}"
        OUTPUT_VARIABLE native_path
        RESULT_VARIABLE rc
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      if (rc EQUAL 0)
        set(${output_var} "${native_path}" PARENT_SCOPE)
      else()
        message(FATAL_ERROR "wslpath failed for '${input_path}'. Ensure wslpath is available.")
      endif()
    else()
      set(${output_var} "${input_path}" PARENT_SCOPE)
    endif()
  endfunction()

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

  if (DOTNET_CAN_BUILD_CLIENT_LIBRARY)
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
  endif()

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

  # ResxGen: .resx -> typed C++ accessors under namespace I18N::<Group>.
  # Discovers groups from filenames (<Group>.<locale>.resx) and emits one
  # .h/.cpp per group plus a master All.h/All.cpp. Generated output lives
  # in the build tree only and is consumed by Main via target_sources.
  set(RESXGEN_PROJ "${CMAKE_CURRENT_SOURCE_DIR}/../Tools/ResxGen/ResxGen.csproj")
  set(RESX_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Localization")
  set(RESX_OUTPUT_DIR "${CMAKE_BINARY_DIR}/Generated/I18N")
  file(MAKE_DIRECTORY "${RESX_OUTPUT_DIR}")

  file(GLOB RESX_FILES CONFIGURE_DEPENDS "${RESX_INPUT_DIR}/*.resx")
  file(GLOB_RECURSE RESXGEN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../Tools/ResxGen/*.cs")

  if (RESX_FILES)
    # Compute groups from filenames: Editor.en.resx -> group "Editor".
    set(RESX_GROUPS "")
    foreach(resx_file IN LISTS RESX_FILES)
      get_filename_component(stem "${resx_file}" NAME_WE)  # Editor.en
      string(REGEX REPLACE "\\.[^.]*$" "" group_name "${stem}")
      list(APPEND RESX_GROUPS "${group_name}")
    endforeach()
    list(REMOVE_DUPLICATES RESX_GROUPS)

    # Predict generator output for the custom command's OUTPUT list.
    set(RESX_GEN_HEADERS "${RESX_OUTPUT_DIR}/All.h")
    set(RESX_GEN_SOURCES "${RESX_OUTPUT_DIR}/All.cpp")
    foreach(group_name IN LISTS RESX_GROUPS)
      list(APPEND RESX_GEN_HEADERS "${RESX_OUTPUT_DIR}/${group_name}.h")
      list(APPEND RESX_GEN_SOURCES "${RESX_OUTPUT_DIR}/${group_name}.cpp")
    endforeach()

    mu_native_path("${RESXGEN_PROJ}" RESXGEN_PROJ_NATIVE)
    mu_native_path("${RESX_INPUT_DIR}" RESX_INPUT_DIR_NATIVE)
    mu_native_path("${RESX_OUTPUT_DIR}" RESX_OUTPUT_DIR_NATIVE)

    add_custom_command(
      OUTPUT ${RESX_GEN_HEADERS} ${RESX_GEN_SOURCES}
      COMMAND ${CMAKE_COMMAND} -E env
              "NUGET_PACKAGES=${DOTNET_NUGET_DIR_NATIVE}"
              "DOTNET_CLI_HOME=${DOTNET_TEMP_DIR_NATIVE}"
              "${DOTNET_EXECUTABLE}" run --project "${RESXGEN_PROJ_NATIVE}" -c $<CONFIG>
              -- --input "${RESX_INPUT_DIR_NATIVE}" --output "${RESX_OUTPUT_DIR_NATIVE}"
              --wide-groups Game,Dialog
      DEPENDS ${RESX_FILES} ${RESXGEN_SOURCES} "${RESXGEN_PROJ}"
      COMMENT "ResxGen: regenerating C++ I18N accessors from .resx"
      VERBATIM
    )

    add_custom_target(ResxGen DEPENDS ${RESX_GEN_HEADERS} ${RESX_GEN_SOURCES})
    add_dependencies(MuClient ResxGen)
    target_sources(MuClient PRIVATE ${RESX_GEN_SOURCES})
    target_include_directories(MuClient PUBLIC "${CMAKE_BINARY_DIR}/Generated")
  endif()
else()
  message(WARNING ".NET SDK not found. MUnique.Client.Library.dll will NOT be built. "
    "The game will run but cannot connect to the server. Install .NET SDK 10.0+ to enable.")
endif()

# MinGW (32-bit or 64-bit) cross-toolchains: link against the Windows/OpenGL
# system libraries so that gl*, wgl*, Imm*, timeGetTime, etc. resolve.
# For turbojpeg and wzAudio we link against the logically named libraries,
# which must be provided as MinGW-compatible .a import libraries.
if (CMAKE_CXX_COMPILER MATCHES "mingw32" OR CMAKE_CXX_COMPILER MATCHES "w64-mingw32")
  target_link_directories(MuClient PUBLIC
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

  target_link_libraries(MuClient PUBLIC
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
  )
endif()

# Link SDL3 + SDL3_mixer (audio) on all platforms.
target_link_libraries(MuClient PUBLIC
  SDL3::SDL3
  SDL3_mixer::SDL3_mixer
)

# Link ImGui for all platforms when editor is enabled. imgui carries its own
# opengl32/dwmapi link dependencies (see its definition above), so the engine
# only needs to link the imgui target itself.
if(ENABLE_EDITOR)
  target_link_libraries(MuClient PUBLIC imgui)
endif()
