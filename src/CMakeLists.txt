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

# SDL3 — static link, no tests, no install
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/SDL EXCLUDE_FROM_ALL)
set(_saved_BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS}")
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(SDL3_ttf)
FetchContent_MakeAvailable(SDL3_image)
set(BUILD_SHARED_LIBS "${_saved_BUILD_SHARED_LIBS}")

# FreeType -- RmlUi's default font engine (below) requires a Freetype::Freetype target.
# Not vendored separately, and no alias needed here either: SDL3_ttf's own CMakeLists.txt
# (FetchContent'd above) already does `if(NOT TARGET Freetype::Freetype)
# add_library(Freetype::Freetype ALIAS freetype)` for its own vendored copy -- confirmed in
# _deps/sdl3_ttf-src/CMakeLists.txt. RmlUi's own Dependencies.cmake only checks
# `if(NOT TARGET Freetype::Freetype)` (CMake/Utilities.cmake's
# report_dependency_found_or_error), so it picks up SDL3_ttf's alias for free. A second
# `add_library(... ALIAS ...)` here collided with it under vcpkg's `_add_library` override
# (CMake error: "cannot create ALIAS target 'Freetype::Freetype' because another target
# with the same name already exists") -- don't re-add it.

mu_ensure_submodule("ThirdParty/RmlUi"
  INDICATOR "CMakeLists.txt"
  URL "https://github.com/mikke89/RmlUi.git")

# RmlUi -- static link, no bundled samples/Lua bindings. We supply our own
# Rml::RenderInterface/SystemInterface (source/Render/RmlUi/) targeting this branch's
# IMuRenderer/SDL_GPU renderer instead of one of RmlUi's stock backends -- except for the
# renderer, which (unlike on feature/rmlui-migration, where RHI:: was a closer match than
# any of RmlUi's own backends) reuses RmlUi's own vendored RenderInterface_SDL_GPU
# (Backends/RmlUi_Renderer_SDL_GPU.cpp, compiled explicitly below) rather than a hand-rolled
# bridge -- see the rmlui-on-sdl-gpu port plan for why. RMLUI_BACKEND only selects which
# backend the SAMPLES build against (RmlUi's own CMakeLists.txt validates it unconditionally
# regardless of RMLUI_SAMPLES though, so it must still be left at a valid value from
# CMake/OptionsLists.cmake; "auto" is a no-op here since RMLUI_SAMPLES is OFF). Font engine
# left at RmlUi's own default (freetype, vendored above) rather than building a custom bridge
# to this engine's text rendering. Not gated behind ENABLE_EDITOR (unlike imgui above): this
# ships in the real game, not just admin/editor builds.
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_BACKEND "auto" CACHE STRING "" FORCE)
set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
set(RMLUI_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/RmlUi EXCLUDE_FROM_ALL)

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

# The in-game shop uses one libcurl transport on every platform. Keep the
# legacy WinINet implementation out of the build.
list(FILTER MU_CLIENT_SOURCES EXCLUDE REGEX "[/\\]GameShop[/\\]FileDownloader[/\\](FileDownloader|FTPConnecter|HTTPConnecter|DownloadInfo)\\.cpp$")

if(WIN32)
  list(FILTER MU_CLIENT_SOURCES EXCLUDE REGEX "[/\\]Core[/\\]Platform[/\\]posix[/\\]")
else()
  list(FILTER MU_CLIENT_SOURCES EXCLUDE REGEX "[/\\]Core[/\\]Platform[/\\]win32[/\\]")
endif()

list(FILTER MU_CLIENT_SOURCES EXCLUDE REGEX "[/\\]Render[/\\]Renderer[/\\]MuRenderer\\.cpp$")

# The downstream renderer is SDL GPU-only. Upstream's retired OpenGL Core
# Profile backend is preserved through the existing Render/Renderer API, not
# compiled as a second backend.
list(FILTER MU_CLIENT_SOURCES EXCLUDE REGEX "[/\\]Render[/\\](Core|RHI|Shaders)[/\\]")

# RmlUi's official SDL input-event backend (RmlSDL::InputEventHandler/ConvertKey/
# ConvertMouseButton/GetKeyModifierState) and SDL_GPU renderer backend
# (RenderInterface_SDL_GPU) -- neither is part of RmlUi::Core itself (RMLUI_SAMPLES=OFF
# above skips building any of RmlUi's bundled renderer/platform backends). Only these two
# files are compiled, not the rest of ThirdParty/RmlUi/Backends/ (which also contains
# GL2/GL3/Vulkan renderer backends and a SystemInterface_SDL class this project doesn't
# need -- see RmlUiSystemInterface). Not picked up by the GLOB_RECURSE above since it globs
# "source/*.cpp", not ThirdParty/.
list(APPEND MU_CLIENT_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/RmlUi/Backends/RmlUi_Platform_SDL.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/RmlUi/Backends/RmlUi_Renderer_SDL_GPU.cpp"
)

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
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_sdl3.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_sdlgpu3.cpp"
  )

  target_include_directories(imgui PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends"
  )

  # The SDL3 ImGui backend (imgui_impl_sdl3.cpp) needs the SDL3 headers; link the
  # target so its include dirs propagate (issue #442).
  target_link_libraries(imgui PUBLIC SDL3::SDL3)

  # <-- Force C++20 for ImGui here
  target_compile_features(imgui PUBLIC cxx_std_20)
endif()

# Engine/game code as a static library. Compile- and link-time usage
# requirements (C++20, include paths, definitions, third-party libraries) are
# attached PUBLIC below so both the Main executable and any test binary that
# links MuClient inherit them automatically.
add_library(MuClient STATIC ${MU_CLIENT_SOURCES})
target_compile_features(MuClient PUBLIC cxx_std_20)
if(MSVC)
  target_compile_options(MuClient PRIVATE /utf-8 /we4828)
endif()
target_compile_definitions(MuClient PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_link_libraries(MuClient PUBLIC SDL3::SDL3 glm::glm spdlog::spdlog)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)
target_link_libraries(MuClient PUBLIC OpenSSL::Crypto)
if(TARGET SDL3_ttf::SDL3_ttf)
  target_link_libraries(MuClient PUBLIC SDL3_ttf::SDL3_ttf)
endif()
if(TARGET SDL3_image::SDL3_image)
  target_link_libraries(MuClient PUBLIC SDL3_image::SDL3_image)
endif()

# RmlUi (game UI middleware). The real target is rmlui_core, aliased as RmlUi::Core
# (Source/Core/CMakeLists.txt).
target_link_libraries(MuClient PUBLIC RmlUi::Core)
target_include_directories(MuClient PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/RmlUi/Backends"
)
# RmlUi_Platform_SDL.h's SDL2-vs-SDL3 API switch -- this project is SDL3-only.
target_compile_definitions(MuClient PUBLIC RMLUI_SDL_VERSION_MAJOR=3)

# The in-game shop downloader uses libcurl on every platform.
find_package(CURL REQUIRED)
target_link_libraries(MuClient PUBLIC CURL::libcurl)

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
  set(MU_APP_ENTRY
    "source/App/Platform/macOS/main.mm"
    "source/App/Platform/Windows/Winmain.cpp")
elseif (ANDROID)
  message(FATAL_ERROR "Android is not a supported build target yet: it needs "
    "an OpenGL ES renderer port, and its entry point is android_main via "
    "native_app_glue built as a shared library, not an executable. When that "
    "work starts, add source/App/Platform/Android/ and replace this branch.")
elseif (UNIX)
  # Linux entry (main.cpp) plus the shared bootstrap/game-loop from Winmain.cpp,
  # which now compiles off Windows (issue #462). main() calls into it.
  set(MU_APP_ENTRY
    "source/App/Platform/Linux/main.cpp"
    "source/App/Platform/Windows/Winmain.cpp")
else()
  message(FATAL_ERROR "No executable entry point defined for this platform "
    "(CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}). Add source/App/Platform/<OS>/ "
    "and a branch above.")
endif()

if (APPLE)
  set(MU_MACOS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/source/App/Platform/macOS/MuMain.icns")
  add_executable(Main MACOSX_BUNDLE
    "${CMAKE_CURRENT_SOURCE_DIR}/${MU_APP_ENTRY}"
    "${MU_MACOS_ICON}"
  )
  set_source_files_properties("${MU_MACOS_ICON}" PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
  )
  set_target_properties(Main PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "MU Online"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.openmu.mumain"
    MACOSX_BUNDLE_ICON_FILE "MuMain.icns"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/source/App/Platform/macOS/Info.plist.in"
  )
else()
  add_executable(Main
    "${CMAKE_CURRENT_SOURCE_DIR}/${MU_APP_ENTRY}"
  )
endif()
target_compile_features(Main PUBLIC cxx_std_20)
target_link_libraries(Main PRIVATE MuClient::MuClient)

if (NOT MSVC)
  set(MU_MAIN_MAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/Main.map")
  if (APPLE)
    target_link_options(Main PRIVATE "-Wl,-map,${MU_MAIN_MAP_FILE}")
  else()
    target_link_options(Main PRIVATE "-Wl,-Map,${MU_MAIN_MAP_FILE}")
  endif()
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
# The custom command's stamp file depends on the selected files under
# ${ASSETS_DIR}, so unchanged builds skip the copy. CONFIGURE_DEPENDS
# makes CMake re-evaluate the glob at build time when files are added.
set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
message(STATUS "=== DEBUG: Assets directory: ${ASSETS_DIR}")
option(MU_COPY_RUNTIME_ASSETS "Copy Data and fonts beside Main" ON)
if(MU_COPY_RUNTIME_ASSETS)
  set(ASSETS_MODE "with-data")
  file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS "${ASSETS_DIR}/*")
else()
  set(ASSETS_MODE "no-data")
  file(GLOB ASSET_FILES LIST_DIRECTORIES FALSE CONFIGURE_DEPENDS "${ASSETS_DIR}/*")
endif()
message(STATUS "=== DEBUG: Runtime asset mode: ${ASSETS_MODE}")
set(ASSETS_MODE_FILE "${CMAKE_CURRENT_BINARY_DIR}/.assets_mode")
file(GENERATE OUTPUT "${ASSETS_MODE_FILE}" CONTENT "${ASSETS_MODE}\n")
# $<CONFIG> in the stamp name keeps multi-config generators (Visual
# Studio, Xcode) honest: each configuration's output directory needs
# its own stamp, otherwise switching from Debug to Release would see
# the Debug stamp and skip the copy, leaving the Release dir empty.
set(ASSETS_STAMP "${CMAKE_CURRENT_BINARY_DIR}/.assets_copied_$<CONFIG>.stamp")
add_custom_command(
  OUTPUT "${ASSETS_STAMP}"
  COMMAND ${CMAKE_COMMAND} -E echo "=== Copying assets from ${ASSETS_DIR} to $<TARGET_FILE_DIR:Main>"
  COMMAND "${CMAKE_COMMAND}"
          "-DMU_ASSET_SOURCE=${ASSETS_DIR}"
          "-DMU_ASSET_OUTPUT=$<TARGET_FILE_DIR:Main>"
          "-DMU_COPY_RUNTIME_ASSETS=${MU_COPY_RUNTIME_ASSETS}"
          "-P" "${REPO_ROOT}/cmake/CopyRuntimeAssets.cmake"
  COMMAND ${CMAKE_COMMAND} -E touch "${ASSETS_STAMP}"
  DEPENDS ${ASSET_FILES} "${ASSETS_MODE_FILE}"
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
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
    "${MU_SHADER_OUTPUT_DIR}"
    "$<TARGET_FILE_DIR:Main>/shaders"
  VERBATIM
)

# Skeleton Linux-native linkage. This intentionally depends only on
# ubiquitous system libraries so we can focus on getting the project to
# compile; more platform-specific adjustments will follow as we iterate.
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_library(TURBOJPEG_LIBRARY NAMES turbojpeg REQUIRED)
  target_link_libraries(MuClient PUBLIC
    pthread
    dl
    m
    ${TURBOJPEG_LIBRARY}
  )
endif()

if (APPLE)
  find_library(TURBOJPEG_LIBRARY NAMES turbojpeg
    PATHS
      "/opt/homebrew/lib"
      "/usr/local/lib"
    REQUIRED
  )
  target_link_libraries(MuClient PUBLIC
    ${TURBOJPEG_LIBRARY}
  )
endif()

# --- .NET / NuGet infrastructure (shared by wire-sizes codegen and ClientLibrary)
# Lifted up here so the wire-sizes block below can resolve the OpenMU packet
# XML from the same NuGet cache the ClientLibrary uses.
# A Linux target needs the native Linux dotnet to Native-AOT-compile the
# linux-x64 .so (a Windows dotnet.exe cannot cross-OS AOT). Everything else
# (Windows/MinGW targets, including from WSL) prefers dotnet.exe so the win-*
# AOT build works. Pick accordingly.
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_program(DOTNET_EXECUTABLE dotnet)
  if (NOT DOTNET_EXECUTABLE)
    find_program(DOTNET_EXECUTABLE dotnet.exe)
  endif()
else()
  find_program(DOTNET_EXECUTABLE dotnet.exe)
  if (NOT DOTNET_EXECUTABLE)
    find_program(DOTNET_EXECUTABLE dotnet)
  endif()
endif()

# Helper macro: convert a path to Windows-native format when using
# Windows dotnet.exe from WSL. On all other platforms this is a no-op.
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

# NuGet cache: defaults to <project>/.nuget, override with -DMU_NUGET_CACHE_DIR=...
set(MU_NUGET_CACHE_DIR "${CMAKE_SOURCE_DIR}/.nuget" CACHE PATH "NuGet package cache directory")
file(MAKE_DIRECTORY "${MU_NUGET_CACHE_DIR}")

# --- Generated wire-size guards ----------------------------------------------
# tools/gen_wire_sizes.py reads OpenMU's authoritative packet XML and emits
# wire_sizes.generated.h with static_asserts that lock each client packet
# struct's sizeof <= the wire length declared by the server. Guards against
# the PR #402 class of bug (silent #pragma pack drift that freezes the client).
#
# The XML lives in the MUnique.OpenMU.Network.Packets NuGet package -- the
# same package the .NET ClientLibrary consumes (see MUnique.Client.Library.csproj).
# Sourcing from NuGet means a single versioned dependency instead of vendoring
# the entire OpenMU repository as a submodule.
set(OPENMU_PACKETS_VERSION "0.9.10" CACHE STRING
    "Version of MUnique.OpenMU.Network.Packets to source packet XML from")
set(OPENMU_PACKETS_XML
    "${MU_NUGET_CACHE_DIR}/munique.openmu.network.packets/${OPENMU_PACKETS_VERSION}/contentFiles/any/net10.0/ServerToClient/ServerToClientPackets.xml")

# Restore the package on demand at configure time if it isn't cached yet.
# dotnet restore is fast on cache-hit, so re-configuring stays cheap.
if (NOT EXISTS "${OPENMU_PACKETS_XML}")
  if (NOT DOTNET_EXECUTABLE)
    message(FATAL_ERROR
      "wire-size codegen needs MUnique.OpenMU.Network.Packets v${OPENMU_PACKETS_VERSION} "
      "but it is not cached at ${MU_NUGET_CACHE_DIR} and no .NET SDK was found to restore "
      "it. Install dotnet (Linux or Windows) or pre-populate the NuGet cache.")
  endif()
  message(STATUS "Restoring MUnique.OpenMU.Network.Packets v${OPENMU_PACKETS_VERSION} for wire-size codegen...")
  set(_packets_csproj "${CMAKE_CURRENT_SOURCE_DIR}/../ClientLibrary/MUnique.Client.Library.csproj")
  mu_native_path("${_packets_csproj}" _packets_csproj_native)
  mu_native_path("${MU_NUGET_CACHE_DIR}" _packets_nuget_native)
  # WSLENV=NUGET_PACKAGES/w is needed so the env var crosses the WSL->Windows
  # interop boundary when DOTNET_EXECUTABLE is dotnet.exe; harmless otherwise.
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
            "WSLENV=NUGET_PACKAGES/w"
            "NUGET_PACKAGES=${_packets_nuget_native}"
            "${DOTNET_EXECUTABLE}" restore "${_packets_csproj_native}" --nologo
    RESULT_VARIABLE _restore_rc
  )
  if (NOT _restore_rc EQUAL 0)
    message(FATAL_ERROR "dotnet restore failed (rc=${_restore_rc}) -- see output above.")
  endif()
  if (NOT EXISTS "${OPENMU_PACKETS_XML}")
    message(FATAL_ERROR
      "dotnet restore completed but ServerToClientPackets.xml is still missing at "
      "${OPENMU_PACKETS_XML}. Did the NuGet package layout change?")
  endif()
endif()

set(WIRE_SIZES_GEN_SCRIPT "${REPO_ROOT}/tools/gen_wire_sizes.py")
set(WIRE_SIZES_GEN_HEADER "${CMAKE_BINARY_DIR}/generated/Network/Server/wire_sizes.generated.h")
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_custom_command(
  OUTPUT "${WIRE_SIZES_GEN_HEADER}"
  COMMAND "${Python3_EXECUTABLE}" "${WIRE_SIZES_GEN_SCRIPT}"
          --xml "${OPENMU_PACKETS_XML}"
          --source-label "MUnique.OpenMU.Network.Packets v${OPENMU_PACKETS_VERSION} (NuGet)"
          --output "${WIRE_SIZES_GEN_HEADER}"
  DEPENDS "${WIRE_SIZES_GEN_SCRIPT}" "${OPENMU_PACKETS_XML}"
  COMMENT "Generating wire_sizes.generated.h from OpenMU NuGet packet XML..."
  VERBATIM
)
add_custom_target(GenWireSizes DEPENDS "${WIRE_SIZES_GEN_HEADER}")
# WSclient.h includes wire_sizes.generated.h, and WSclient.cpp compiles into the
# MuClient library, not Main. The codegen ordering must therefore hang off
# MuClient so the header is generated before any consuming TU compiles.
# Attaching it to Main only ordered codegen before Main's own link step, which
# let a parallel `cmake --build build` compile MuClient sources before the
# header existed and fail (workaround was a manual --target GenWireSizes first).
add_dependencies(MuClient GenWireSizes)

target_include_directories(MuClient PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/source"
  # source/App holds the moved PCH header (stdafx.h). Keeping App/ on the
  # include path lets the ~350 bare #include "stdafx.h" resolve unchanged
  # after the bootstrap files moved out of source/ (issue #346).
  "${CMAKE_CURRENT_SOURCE_DIR}/source/App"
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include"
  "${miniaudio_SOURCE_DIR}"
  "${miniaudio_SOURCE_DIR}/extras"
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/netcore/includes"
  "${CMAKE_BINARY_DIR}/generated"
  # stb_truetype backs the portable GDI text rasterizer (issue #462).
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb"
  $<$<BOOL:${ENABLE_EDITOR}>:${CMAKE_CURRENT_SOURCE_DIR}/MuEditor>
)

set_source_files_properties(
  "${CMAKE_CURRENT_SOURCE_DIR}/source/Core/Platform/Audio/MiniAudioImpl.cpp"
  PROPERTIES
    SKIP_PRECOMPILE_HEADERS ON
    COMPILE_OPTIONS "$<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>:-Wno-error=tautological-compare>"
)

# Basic compile definitions, roughly mirroring the original MSVC project
# (fine-tuning per configuration can be done later if needed.)
target_compile_definitions(MuClient PUBLIC
  _LANGUAGE_FOREIGN
  _LANGUAGE_ENG
  UNICODE
  "MU_DOTNET_LIB_EXT=\"${CMAKE_SHARED_LIBRARY_SUFFIX}\""
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
endif()

if (MSVC)
  # ponytail: native CI uses dynamic vcpkg triplets; guard an empty runtime
  # DLL list if static triplets are added.
  add_custom_command(TARGET Main POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E "$<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:Main>>,copy_if_different,true>"
      $<TARGET_RUNTIME_DLLS:Main>
      "$<TARGET_FILE_DIR:Main>"
    COMMAND_EXPAND_LISTS
    COMMENT "Staging linked-target runtime DLLs"
    VERBATIM
  )
endif()

# .NET Client Library and tools (platform-agnostic, requires dotnet SDK)
# DOTNET_EXECUTABLE, mu_native_path() and MU_NUGET_CACHE_DIR are already
# resolved above (shared with the wire-size codegen block).
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

  # 1. Define the output path. Native AOT emits a platform-native shared
  # library: .dll on Windows, .so on Linux (the runtime client dlopen's it by
  # this name).
  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(DOTNET_LIB_NAME "MUnique.Client.Library.so")
  elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DOTNET_LIB_NAME "MUnique.Client.Library.dylib")
  else()
    set(DOTNET_LIB_NAME "MUnique.Client.Library.dll")
  endif()
  set(DOTNET_DLL_PATH "${CMAKE_CURRENT_BINARY_DIR}/${DOTNET_LIB_NAME}")
  set(DOTNET_PROJ "${CMAKE_CURRENT_SOURCE_DIR}/../ClientLibrary/MUnique.Client.Library.csproj")

  # Create temp directories for Native AOT build.
  set(DOTNET_TEMP_OUTPUT "${CMAKE_BINARY_DIR}/dotnet_out")
  set(DOTNET_TEMP_DIR "${CMAKE_BINARY_DIR}/.dotnet_temp")
  set(DOTNET_NUGET_DIR "${MU_NUGET_CACHE_DIR}")
  file(MAKE_DIRECTORY "${DOTNET_TEMP_OUTPUT}")
  file(MAKE_DIRECTORY "${DOTNET_TEMP_DIR}")

  # Convert paths to Windows-native format for MSBuild (no-op outside WSL).
  mu_native_path("${DOTNET_PROJ}" DOTNET_PROJ_NATIVE)
  mu_native_path("${DOTNET_TEMP_OUTPUT}" DOTNET_TEMP_OUTPUT_NATIVE)
  mu_native_path("${DOTNET_NUGET_DIR}" DOTNET_NUGET_DIR_NATIVE)
  mu_native_path("${DOTNET_TEMP_DIR}" DOTNET_TEMP_DIR_NATIVE)

  # 2. Tell CMake to watch ALL .cs files in the ClientLibrary folder
  file(GLOB_RECURSE DOTNET_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../ClientLibrary/*.cs")

  # 3. Build to temp directory with Native AOT, then copy to build and bin directories
  # Override environment variables to avoid issues with umlauts in Windows username
  # Determine the runtime identifier from the target OS and pointer width. Native
  # AOT has no linux-x86 target, so 32-bit is Windows-only.
  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(DOTNET_RID "linux-x64")
    set(DOTNET_PLATFORM "x64")
    # The PreBuild XSLT step regenerates files using Windows-style backslash
    # paths and corrupts them on Linux; the generated sources are committed, so
    # skip it (ci=true) and use the checked-in output.
    set(DOTNET_EXTRA_ARGS "-p:ci=true")
    message(STATUS ".NET Client Library will build for linux-x64")
  elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if (CMAKE_OSX_ARCHITECTURES MATCHES "(^|;)arm64($|;)" OR CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
      set(DOTNET_RID "osx-arm64")
      set(DOTNET_PLATFORM "arm64")
    else()
      set(DOTNET_RID "osx-x64")
      set(DOTNET_PLATFORM "x64")
    endif()
    set(DOTNET_EXTRA_ARGS "-p:ci=true")
    message(STATUS ".NET Client Library will build for ${DOTNET_RID}")
  elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DOTNET_RID "win-x64")
    set(DOTNET_PLATFORM "x64")
    set(DOTNET_EXTRA_ARGS "")
    message(STATUS ".NET Client Library will build for win-x64 (CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})")
  else()
    set(DOTNET_RID "win-x86")
    set(DOTNET_PLATFORM "x86")
    set(DOTNET_EXTRA_ARGS "")
    message(STATUS ".NET Client Library will build for win-x86 (CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})")
  endif()

  if (DOTNET_CAN_BUILD_CLIENT_LIBRARY)
    target_compile_definitions(MuClient PRIVATE MU_HAS_DOTNET_AOT)

    set(DOTNET_PUBLISH_ENV
      "NUGET_PACKAGES=${DOTNET_NUGET_DIR_NATIVE}"
      "DOTNET_CLI_HOME=${DOTNET_TEMP_DIR_NATIVE}"
      "TEMP=${DOTNET_TEMP_DIR_NATIVE}"
      "TMP=${DOTNET_TEMP_DIR_NATIVE}"
    )

    if (UNIX AND OPENSSL_CRYPTO_LIBRARY)
      get_filename_component(MU_OPENSSL_LIBRARY_PATH "${OPENSSL_CRYPTO_LIBRARY}" DIRECTORY)
      if (DEFINED ENV{LIBRARY_PATH} AND NOT "$ENV{LIBRARY_PATH}" STREQUAL "")
        string(APPEND MU_OPENSSL_LIBRARY_PATH ":$ENV{LIBRARY_PATH}")
      endif()
      list(APPEND DOTNET_PUBLISH_ENV "LIBRARY_PATH=${MU_OPENSSL_LIBRARY_PATH}")
    endif()

    add_custom_command(
            OUTPUT "${DOTNET_DLL_PATH}"
            COMMAND ${CMAKE_COMMAND} -E echo "--- C# Changes Detected: Rebuilding Client Library ---"
            COMMAND ${CMAKE_COMMAND} -E env
                    ${DOTNET_PUBLISH_ENV}
                    "${DOTNET_EXECUTABLE}" publish "${DOTNET_PROJ_NATIVE}" -c $<CONFIG> -r ${DOTNET_RID} -p:PlatformTarget=${DOTNET_PLATFORM} ${DOTNET_EXTRA_ARGS} -o "${DOTNET_TEMP_OUTPUT_NATIVE}" --nologo
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DOTNET_TEMP_OUTPUT}/${DOTNET_LIB_NAME}" "${DOTNET_DLL_PATH}"
            DEPENDS "${DOTNET_PROJ}" ${DOTNET_SOURCES}
            COMMENT "Checking for .NET Client Library updates..."
            VERBATIM
    )

    # 4. Build and stage the library whenever Main's dependency is evaluated.
    # Main may not relink after a C#-only change, so TARGET Main POST_BUILD is
    # not sufficient to keep the runtime copy current.
    add_custom_target(ClientLibrary
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DOTNET_DLL_PATH}"
        "$<TARGET_FILE_DIR:Main>/${DOTNET_LIB_NAME}"
      DEPENDS "${DOTNET_DLL_PATH}"
      COMMENT "Staging ClientLibrary in the Main runtime directory"
      VERBATIM
    )
    add_dependencies(Main ClientLibrary)
    set_target_properties(ClientLibrary PROPERTIES EXCLUDE_FROM_ALL TRUE)
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
  set(RESXGEN_PROJ "${CMAKE_CURRENT_SOURCE_DIR}/../tools/ResxGen/ResxGen.csproj")
  set(RESX_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Localization")
  set(RESX_OUTPUT_DIR "${CMAKE_BINARY_DIR}/Generated/I18N")
  file(MAKE_DIRECTORY "${RESX_OUTPUT_DIR}")

  file(GLOB RESX_FILES CONFIGURE_DEPENDS "${RESX_INPUT_DIR}/*.resx")
  file(GLOB_RECURSE RESXGEN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../tools/ResxGen/*.cs")

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
      # -c Release (NOT $<CONFIG>) is deliberate: RESX_OUTPUT_DIR is shared between the Debug and
      # Release C++ builds (not config-suffixed), but Ninja Multi-Config generates a separate rule
      # per config for whatever command produces a given OUTPUT. With $<CONFIG> here, building
      # Debug then Release (build-all.ps1's default) recorded two different command lines for the
      # same output paths -- Ninja saw each config's rule as "the command that produced this
      # output changed" and reran ResxGen every single invocation, and CppEmitter.cs's unconditional
      # File.WriteAllText bumped every generated I18N file's mtime, cascading into a near-full
      # rebuild of every TU that includes an I18N header (most of the tree). The generated I18N
      # C++ source content never actually depended on which .NET config built the *generator*, so
      # pinning it to one fixed config removes the config-dependence entirely instead of fixing it
      # per-config (which would just duplicate identical output under Debug/Release subdirs).
      COMMAND ${CMAKE_COMMAND} -E env
              --unset=Platform
              "NUGET_PACKAGES=${DOTNET_NUGET_DIR_NATIVE}"
              "DOTNET_CLI_HOME=${DOTNET_TEMP_DIR_NATIVE}"
              "${DOTNET_EXECUTABLE}" run --project "${RESXGEN_PROJ_NATIVE}" -c Release
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

# DXP-10 step 4: state-wrapper monopoly guard. Fails the build if a raw gl*()
# call appears outside Render/ that isn't on tools/check_gl_wrapper_monopoly.py's
# allowlist (which mirrors DXP-10-state-wrapper-monopoly.md's documented, deliberate
# deferrals -- DXP-12 texture territory, DXP-14 fog params, debug/editor files).
# No OUTPUT means this target is always considered out-of-date and re-runs every build.
add_custom_target(CheckGLWrapperMonopoly ALL
  COMMAND "${Python3_EXECUTABLE}" "${REPO_ROOT}/tools/check_gl_wrapper_monopoly.py"
          --source-root "${CMAKE_CURRENT_SOURCE_DIR}/source"
  COMMENT "DXP-10: checking for raw gl* calls outside Render/ (state-wrapper monopoly guard)"
  VERBATIM
)
add_dependencies(MuClient CheckGLWrapperMonopoly)

# No OUTPUT means this target is always considered out-of-date and re-runs every build. See
# tools/check_rml_rcss_syntax.py's own docstring for what it does and doesn't check.
add_custom_target(CheckRmlRcssSyntax ALL
  COMMAND "${Python3_EXECUTABLE}" "${REPO_ROOT}/tools/check_rml_rcss_syntax.py"
          --asset-root "${ASSETS_DIR}/Data/Interface/RmlUi"
  COMMENT "Checking RML/RCSS syntax (comment nesting, brace balance)"
  VERBATIM
)
add_dependencies(Main CheckRmlRcssSyntax)

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

if(APPLE)
  target_link_libraries(MuClient PUBLIC
    "-framework AudioToolbox"
    "-framework AudioUnit"
    "-framework CoreAudio"
    "-framework CoreFoundation"
  )
endif()

# Link ImGui for all platforms when editor is enabled. Untouched compatibility
# code retains its existing OpenGL system-library links above.
if(ENABLE_EDITOR)
  target_link_libraries(MuClient PUBLIC imgui)
endif()
