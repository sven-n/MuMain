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

# ============================================================
# ImGui static library (required when editor is enabled)
# ============================================================
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

  target_compile_features(imgui PUBLIC cxx_std_20)
endif()

# ============================================================
# Common settings shared across all module library targets.
# INTERFACE library propagates include dirs, definitions, and
# C++20 to every target that links against it.
# ============================================================
set(MU_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")

add_library(MUCommon INTERFACE)
target_compile_features(MUCommon INTERFACE cxx_std_20)

target_include_directories(MUCommon INTERFACE
  "${MU_SOURCE_DIR}"
  "${MU_SOURCE_DIR}/Main"
  "${MU_SOURCE_DIR}/Core"
  "${MU_SOURCE_DIR}/Protocol"
  "${MU_SOURCE_DIR}/Network"
  "${MU_SOURCE_DIR}/Data"
  "${MU_SOURCE_DIR}/Data/Items"
  "${MU_SOURCE_DIR}/Data/Skills"
  "${MU_SOURCE_DIR}/World"
  "${MU_SOURCE_DIR}/World/Maps"
  "${MU_SOURCE_DIR}/Gameplay"
  "${MU_SOURCE_DIR}/Gameplay/Characters"
  "${MU_SOURCE_DIR}/Gameplay/Items"
  "${MU_SOURCE_DIR}/Gameplay/Pets"
  "${MU_SOURCE_DIR}/Gameplay/Events"
  "${MU_SOURCE_DIR}/Gameplay/Buffs"
  "${MU_SOURCE_DIR}/Gameplay/Skills"
  "${MU_SOURCE_DIR}/Gameplay/Social"
  "${MU_SOURCE_DIR}/Gameplay/NPCs"
  "${MU_SOURCE_DIR}/UI/Framework"
  "${MU_SOURCE_DIR}/UI/Windows"
  "${MU_SOURCE_DIR}/UI/Windows/Events"
  "${MU_SOURCE_DIR}/UI/Windows/Inventory"
  "${MU_SOURCE_DIR}/UI/Windows/HUD"
  "${MU_SOURCE_DIR}/UI/Windows/Commerce"
  "${MU_SOURCE_DIR}/UI/Windows/Social"
  "${MU_SOURCE_DIR}/UI/Windows/Castle"
  "${MU_SOURCE_DIR}/UI/Windows/Character"
  "${MU_SOURCE_DIR}/UI/Windows/Quest"
  "${MU_SOURCE_DIR}/UI/Events"
  "${MU_SOURCE_DIR}/UI/Legacy"
  "${MU_SOURCE_DIR}/Audio"
  "${MU_SOURCE_DIR}/RenderFX"
  "${MU_SOURCE_DIR}/ThirdParty"
  "${MU_SOURCE_DIR}/Resources/Windows"
  "${MU_SOURCE_DIR}/Platform"
  "${MU_SOURCE_DIR}/Scenes"
  "${MU_SOURCE_DIR}/Dotnet"
  "${MU_SOURCE_DIR}/GameShop"
  "${MU_SOURCE_DIR}/Translation"
  # Story 7.6.1: Non-Windows stub headers for Windows CRT/SDK headers (process.h, crtdbg.h, etc.)
  # Must come BEFORE dependencies/include so compat-headers/GL/gl.h is found before
  # the vendored Windows-case gl/GL.h on case-insensitive macOS filesystems.
  # [VS0-QUAL-BUILDCOMP-MACOS]
  $<$<NOT:$<BOOL:${WIN32}>>:${MU_SOURCE_DIR}/Platform/compat-headers>
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/netcore/includes"
  $<$<BOOL:${ENABLE_EDITOR}>:${CMAKE_CURRENT_SOURCE_DIR}/MuEditor>
)

target_compile_definitions(MUCommon INTERFACE
  _LANGUAGE_FOREIGN
  _LANGUAGE_ENG
  UNICODE
  $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},4>:_USE_32BIT_TIME_T>
  $<$<CONFIG:Debug>:_DEBUG;_FOREIGN_DEBUG>
  $<$<CONFIG:Release>:NDEBUG;_FOREIGN_NDEBUG;LDS_PATCH_GLOBAL_100520>
  $<$<BOOL:${ENABLE_EDITOR}>:_EDITOR>
)

# ============================================================
# Compiler warning configuration
# ============================================================
if(MSVC)
  target_compile_options(MUCommon INTERFACE
    /W4
    /WX
  )
else()
  target_compile_options(MUCommon INTERFACE
    -Wall
    -Wextra
    -Werror
    # Suppress cross-compiler noise from MSVC-specific pragmas (14+ headers)
    -Wno-unknown-pragmas
    # Virtual interface contracts intentionally leave parameters unnamed (100+ instances)
    -Wno-unused-parameter
    # CUITextListBox and derived classes intentionally hide base AddText() overloads
    -Wno-overloaded-virtual
    # Win32 GetProcAddress casts require function-type reinterpretation
    -Wno-cast-function-type
    # False positive on AdjustHeroHeight() and similar conditionally-used functions
    -Wno-unused-function
    # Harmless const qualifiers on primitive return types (17+ instances)
    -Wno-ignored-qualifiers
    # Legacy memset(ptr, NULL, size) pattern in packet structs (GCC only)
    $<$<CXX_COMPILER_ID:GNU>:-Wno-conversion-null>
    # Manual sizeof(wchar_t)*N in memset — matches actual buffer size (GCC only)
    $<$<CXX_COMPILER_ID:GNU>:-Wno-memset-elt-size>
    # Clang: Suppress high-volume legacy UI 'override' warnings (Story 3.1.2)
    $<$<CXX_COMPILER_ID:Clang>:-Wno-inconsistent-missing-override>
    $<$<CXX_COMPILER_ID:AppleClang>:-Wno-inconsistent-missing-override>
    # GCC -O2/-O3 false positives from inlined wmemcpy/std::wstring (MinGW wchar.h)
    $<$<CXX_COMPILER_ID:GNU>:-Wno-array-bounds>
    $<$<CXX_COMPILER_ID:GNU>:-Wno-stringop-overread>
  )
endif()

# ============================================================
# SDL3 dependency (required by MUPlatform and MURenderFX)
# ============================================================
option(MU_ENABLE_SDL3 "Enable SDL3 dependency (disable for MinGW CI if needed)" ON)
if(MU_ENABLE_SDL3)
    # Story 7.3.0: Propagate MU_ENABLE_SDL3 to ALL targets at project scope.
    # Previously only MUPlatform received this flag (PRIVATE), leaving MUCore, MUData,
    # MURenderFX, MUProtocol, and MUAudio without it — causing compilation failures
    # on macOS/Linux where #ifdef MU_ENABLE_SDL3 guards were not active.
    # [VS0-QUAL-BUILDCOMPAT-MACOS]
    add_compile_definitions(MU_ENABLE_SDL3)
    include(FetchContent)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.2.8
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(SDL3)
    # Story 7.3.0: Make SDL3 headers available to ALL targets.
    # PlatformCompat.h (included via PCH) has #ifdef MU_ENABLE_SDL3 sections that
    # #include <SDL3/SDL.h>. Now that MU_ENABLE_SDL3 is project-scope, every target's
    # PCH needs the SDL3 include path. [VS0-QUAL-BUILDCOMPAT-MACOS]
    include_directories(SYSTEM "${sdl3_SOURCE_DIR}/include")
endif()

# ============================================================
# Independent library targets (enforced dependency boundaries)
# ============================================================

# Foundation — no game dependencies
file(GLOB MU_CORE_SOURCES CONFIGURE_DEPENDS "${MU_SOURCE_DIR}/Core/*.cpp")
add_library(MUCore STATIC ${MU_CORE_SOURCES})
target_link_libraries(MUCore PUBLIC MUCommon)
target_precompile_headers(MUCore PRIVATE "${MU_SOURCE_DIR}/Main/stdafx.h")

# Story 7.6.3: OpenSSL for cross-platform credential encryption (mu_encrypt_blob/mu_decrypt_blob)
# Replaces Windows DPAPI with AES-256-GCM via OpenSSL on ALL platforms.
# If OpenSSL is not found, PlatformCompat.h falls back to identity no-op with a log warning.
find_package(OpenSSL QUIET COMPONENTS Crypto)
if(OpenSSL_FOUND)
    target_link_libraries(MUCore PRIVATE OpenSSL::Crypto)
    target_compile_definitions(MUCore PUBLIC MU_HAS_OPENSSL=1)
    message(STATUS "OpenSSL found: mu_encrypt_blob/mu_decrypt_blob will use AES-256-GCM")
else()
    message(STATUS "OpenSSL NOT found: mu_encrypt_blob/mu_decrypt_blob will use identity no-op fallback")
endif()

# Crypto / packet encoding — depends on Core only
file(GLOB MU_PROTOCOL_SOURCES CONFIGURE_DEPENDS "${MU_SOURCE_DIR}/Protocol/*.cpp")
add_library(MUProtocol STATIC ${MU_PROTOCOL_SOURCES})
target_link_libraries(MUProtocol PRIVATE MUCore)
target_precompile_headers(MUProtocol REUSE_FROM MUCore)

# Data loading, localization, game data — depends on Core only
file(GLOB_RECURSE MU_DATA_SOURCES CONFIGURE_DEPENDS "${MU_SOURCE_DIR}/Data/*.cpp")
add_library(MUData STATIC ${MU_DATA_SOURCES})
target_link_libraries(MUData PRIVATE MUCore)
target_precompile_headers(MUData REUSE_FROM MUCore)

# Effects, models, textures, OpenGL — depends on Core only
file(GLOB MU_RENDERFX_SOURCES CONFIGURE_DEPENDS "${MU_SOURCE_DIR}/RenderFX/*.cpp")
# Story 7.6.1: ZzzOpenglUtil.cpp uses WGL (Windows OpenGL extensions) for VSync/MSAA.
# On macOS, WGL stubs in PlatformCompat.h allow compilation; VSync init detects
# "not available" and no-ops. The file provides essential rendering globals/functions.
# [VS0-QUAL-BUILDCOMP-MACOS]
add_library(MURenderFX STATIC ${MU_RENDERFX_SOURCES})
target_link_libraries(MURenderFX PRIVATE MUCore)
if(MU_ENABLE_SDL3)
    target_link_libraries(MURenderFX PRIVATE SDL3::SDL3-static)
endif()
target_precompile_headers(MURenderFX REUSE_FROM MUCore)

# Sound system — depends on Core only
# Story 5.1.1: MuAudio Abstraction Layer [VS1-AUDIO-ABSTRACT-CORE]
# MiniAudio backend files are in Platform/MiniAudio/ — not covered by the Audio/*.cpp glob.
# They are added explicitly below via target_sources.
file(GLOB MU_AUDIO_SOURCES CONFIGURE_DEPENDS "${MU_SOURCE_DIR}/Audio/*.cpp")
add_library(MUAudio STATIC ${MU_AUDIO_SOURCES})

# Add miniaudio backend sources explicitly (outside the Audio/ glob)
target_sources(MUAudio PRIVATE
    "${MU_SOURCE_DIR}/Platform/MiniAudio/MiniAudioBackend.cpp"
    "${MU_SOURCE_DIR}/Platform/MiniAudio/MiniAudioImpl.cpp"
)

# MiniAudioImpl.cpp defines MINIAUDIO_IMPLEMENTATION — it MUST NOT use the PCH,
# otherwise the macro expansion conflicts with the precompiled header symbols.
set_source_files_properties(
    "${MU_SOURCE_DIR}/Platform/MiniAudio/MiniAudioImpl.cpp"
    PROPERTIES
        SKIP_PRECOMPILE_HEADERS ON
        COMPILE_OPTIONS "-Wno-error=tautological-compare"
)

# Include paths: miniaudio vendored headers + MiniAudio backend directory
target_include_directories(MUAudio PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/miniaudio"
    "${MU_SOURCE_DIR}/Platform/MiniAudio"
)

# Threading: miniaudio uses pthreads on Linux/macOS
find_package(Threads REQUIRED)
target_link_libraries(MUAudio PRIVATE MUCore Threads::Threads)

# Linux: miniaudio uses dlopen() internally — link libdl
if(UNIX AND NOT APPLE)
    target_link_libraries(MUAudio PRIVATE dl)
endif()

# macOS: miniaudio uses CoreAudio — link required frameworks
if(APPLE)
    target_link_libraries(MUAudio PRIVATE
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework CoreAudio"
        "-framework CoreFoundation"
    )
endif()

target_precompile_headers(MUAudio REUSE_FROM MUCore)

# External code (regkey, xstreambuf, UIControls)
file(GLOB MU_THIRDPARTY_SOURCES CONFIGURE_DEPENDS "${MU_SOURCE_DIR}/ThirdParty/*.cpp")
add_library(MUThirdParty STATIC ${MU_THIRDPARTY_SOURCES})
target_link_libraries(MUThirdParty PRIVATE MUCommon)
target_precompile_headers(MUThirdParty REUSE_FROM MUCore)
# ThirdParty code: keep warnings visible but don't fail the build
target_compile_options(MUThirdParty PRIVATE -Wno-error)

# Platform abstraction with platform-conditional backend selection
file(GLOB MU_PLATFORM_SOURCES CONFIGURE_DEPENDS "${MU_SOURCE_DIR}/Platform/*.cpp")
# MuPlatform.cpp is included unconditionally via the GLOB above (platform facade)
if(WIN32)
  list(APPEND MU_PLATFORM_SOURCES "${MU_SOURCE_DIR}/Platform/win32/PlatformLibrary.cpp")
  if(MU_ENABLE_SDL3)
    # SDL3 backend replaces Win32 window/event loop when enabled
    list(APPEND MU_PLATFORM_SOURCES
      "${MU_SOURCE_DIR}/Platform/sdl3/SDLWindow.cpp"
      "${MU_SOURCE_DIR}/Platform/sdl3/SDLEventLoop.cpp"
    )
  else()
    list(APPEND MU_PLATFORM_SOURCES "${MU_SOURCE_DIR}/Platform/win32/Win32Window.cpp")
    list(APPEND MU_PLATFORM_SOURCES "${MU_SOURCE_DIR}/Platform/win32/Win32EventLoop.cpp")
  endif()
else()
  list(APPEND MU_PLATFORM_SOURCES "${MU_SOURCE_DIR}/Platform/posix/PlatformLibrary.cpp")
  # Story 7.1.2: POSIX Signal Handlers for Crash Diagnostics [VS0-QUAL-SIGNAL-HANDLERS]
  list(APPEND MU_PLATFORM_SOURCES "${MU_SOURCE_DIR}/Platform/posix/PosixSignalHandlers.cpp")
  if(MU_ENABLE_SDL3)
    list(APPEND MU_PLATFORM_SOURCES
      "${MU_SOURCE_DIR}/Platform/sdl3/SDLWindow.cpp"
      "${MU_SOURCE_DIR}/Platform/sdl3/SDLEventLoop.cpp"
    )
  endif()
endif()
add_library(MUPlatform STATIC ${MU_PLATFORM_SOURCES})
target_link_libraries(MUPlatform PRIVATE MUCore)
# Story 7.6.3: PlatformCrypto.cpp needs OpenSSL headers and link symbols
if(OpenSSL_FOUND)
    target_link_libraries(MUPlatform PRIVATE OpenSSL::Crypto)
endif()
if(MU_ENABLE_SDL3)
    target_link_libraries(MUPlatform PRIVATE SDL3::SDL3-static)
    target_compile_definitions(MUPlatform PRIVATE MU_ENABLE_SDL3)
    # Story 2.2.1: keyboard state array + unmapped-VK logger [VS1-SDL-INPUT-KEYBOARD]
    target_sources(MUPlatform PRIVATE "${MU_SOURCE_DIR}/Platform/sdl3/SDLKeyboardState.cpp")
endif()
target_precompile_headers(MUPlatform REUSE_FROM MUCore)
if(NOT WIN32)
  target_link_libraries(MUPlatform PRIVATE dl)
endif()

# ============================================================
# Coupled library target (honest about mutual dependencies)
# Network ↔ UI, Gameplay → RenderFX, etc. stay together until
# decoupled during the SDL3 migration phases.
# ============================================================
file(GLOB_RECURSE MU_GAME_SOURCES CONFIGURE_DEPENDS
  "${MU_SOURCE_DIR}/Network/*.cpp"
  "${MU_SOURCE_DIR}/World/*.cpp"
  "${MU_SOURCE_DIR}/Gameplay/*.cpp"
  "${MU_SOURCE_DIR}/UI/*.cpp"
  "${MU_SOURCE_DIR}/Scenes/*.cpp"
  "${MU_SOURCE_DIR}/Dotnet/*.cpp"
  "${MU_SOURCE_DIR}/GameShop/*.cpp"
  "${MU_SOURCE_DIR}/Translation/*.cpp"
)
# Story 7.6.1: GameShop/FileDownloader/ implements WinINet FTP/HTTP downloads —
# entirely Windows-specific (replaced by IPlatformHTTP in EPIC-5 Phase 5.7).
# [VS0-QUAL-BUILDCOMP-MACOS]
if(NOT WIN32)
    # Story 7.6.1: GameShop/FileDownloader/ implements WinINet FTP/HTTP downloads —
    # entirely Windows-specific (replaced by IPlatformHTTP in EPIC-5 Phase 5.7).
    # [VS0-QUAL-BUILDCOMP-MACOS]
    list(FILTER MU_GAME_SOURCES EXCLUDE REGEX ".*/GameShop/FileDownloader/.*")
    # Story 7.6.1: ZzzLodTerrain.cpp uses GL/glu.h — compat headers redirect to
    # macOS OpenGL framework. File provides terrain rendering used throughout game.
endif()
add_library(MUGame STATIC ${MU_GAME_SOURCES})
target_link_libraries(MUGame PUBLIC
  MUCore MUProtocol MUData MURenderFX MUAudio MUPlatform MUThirdParty
)
target_precompile_headers(MUGame REUSE_FROM MUCore)

# Story 7.6.1: Legacy game code — Clang-specific warnings that MSVC/MinGW tolerate.
# Keep visible as warnings for gradual cleanup; don't block macOS native build.
if(NOT MSVC)
  set(MU_LEGACY_WARNING_RELAXATIONS
    -Wno-error=deprecated-anon-enum-enum-conversion
    -Wno-error=deprecated-enum-enum-conversion
    -Wno-error=deprecated-enum-compare
    -Wno-error=switch
    -Wno-error=unused-variable
    -Wno-error=unused-but-set-variable
    -Wno-error=unused-private-field
    -Wno-error=null-conversion
    -Wno-error=null-arithmetic
    -Wno-error=logical-op-parentheses
    -Wno-error=sign-compare
    -Wno-error=missing-braces
    -Wno-error=reorder-ctor
    -Wno-error=pointer-bool-conversion
    -Wno-error=tautological-constant-out-of-range-compare
    -Wno-error=tautological-overlap-compare
    -Wno-error=sizeof-pointer-memaccess
    -Wno-error=missing-field-initializers
    -Wno-error=char-subscripts
    -Wno-error=nontrivial-memcall
    -Wno-error=delete-non-abstract-non-virtual-dtor
    -Wno-error=pragma-once-outside-header
    -Wno-error=int-in-bool-context
    -Wno-error=bitwise-op-parentheses
    -Wno-error=sometimes-uninitialized
    -Wno-error=extern-initializer
    -Wno-error=tautological-pointer-compare
    -Wno-error=c++11-narrowing
    -Wno-error=writable-strings
    -Wno-error=constant-conversion
  )
  target_compile_options(MUGame PRIVATE ${MU_LEGACY_WARNING_RELAXATIONS})
endif()

# Editor sources compiled as part of MUGame when enabled
if(ENABLE_EDITOR)
  file(GLOB_RECURSE MU_EDITOR_SOURCES CONFIGURE_DEPENDS "MuEditor/*.cpp")
  target_sources(MUGame PRIVATE ${MU_EDITOR_SOURCES})
endif()

# ============================================================
# Main executable — entry point only
# ============================================================
file(GLOB MU_MAIN_ENTRY_SOURCES CONFIGURE_DEPENDS "${MU_SOURCE_DIR}/Main/*.cpp")
add_executable(Main ${MU_MAIN_ENTRY_SOURCES})
target_link_libraries(Main PRIVATE MUGame)
target_precompile_headers(Main REUSE_FROM MUCore)
if(NOT MSVC)
  target_compile_options(Main PRIVATE ${MU_LEGACY_WARNING_RELAXATIONS})
endif()

# Story 5.2.1: Winmain.cpp includes MiniAudioBackend.h and miniaudio.h directly.
# These paths are PRIVATE to MUAudio (not propagated via MUGame/MUAudio link chain),
# so they must be added to the Main target explicitly for include resolution.
target_include_directories(Main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/miniaudio"
    "${MU_SOURCE_DIR}/Platform/MiniAudio"
)

if (NOT MSVC AND NOT APPLE)
  set(MU_MAIN_MAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/Main.map")
  target_link_options(Main PRIVATE "-Wl,-Map,${MU_MAIN_MAP_FILE}")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set_target_properties(Main PROPERTIES
    WIN32_EXECUTABLE TRUE
  )

  enable_language(RC)
  target_sources(Main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Resources/Windows/resource.rc"
  )

  add_custom_command(
    TARGET Main
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSETS_DIR}"
            "$<TARGET_FILE_DIR:Main>"
    COMMENT "Copying game assets to executable directory"
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(Main PRIVATE
    pthread
    dl
    m
  )
endif()

# Native macOS/Linux builds: link turbojpeg from system (Homebrew on macOS).
# MinGW cross-builds handle turbojpeg separately below.
# [VS0-QUAL-BUILDCOMP-MACOS]
if (NOT WIN32 AND NOT CMAKE_CXX_COMPILER MATCHES "mingw32" AND NOT CMAKE_CXX_COMPILER MATCHES "w64-mingw32")
  find_library(MU_NATIVE_TURBOJPEG turbojpeg
    HINTS /opt/homebrew/lib /usr/local/lib /usr/lib
  )
  if (MU_NATIVE_TURBOJPEG)
    target_link_libraries(Main PRIVATE ${MU_NATIVE_TURBOJPEG})
  else()
    message(WARNING "libturbojpeg not found. Install jpeg-turbo (brew install jpeg-turbo) for native builds.")
  endif()
endif()

# Native macOS builds: link OpenGL framework (legacy GL used by game engine).
# [VS0-QUAL-BUILDCOMP-MACOS]
if (APPLE)
  target_link_libraries(Main PRIVATE
    "-framework OpenGL"
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreFoundation"
  )
endif()

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
    # wzAudio removed — Story 5.2.1: BGM replaced by miniaudio via g_platformAudio (MiniAudioBackend)
  )

  # Locate required DLLs in bin directory
  set(DLL_SEARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set(REQUIRED_DLL_NAMES glew32 ogg vorbisfile)  # wzAudio removed — Story 5.2.1
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

  # Legacy ClientLibrary build — skipped when MU_ENABLE_DOTNET=ON (FindDotnetAOT handles it)
  if(NOT MU_ENABLE_DOTNET)
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
  endif() # NOT MU_ENABLE_DOTNET

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
# For turbojpeg we link against the logically named library,
# which must be provided as MinGW-compatible .a import libraries.
# wzAudio removed — Story 5.2.1: BGM replaced by miniaudio via g_platformAudio (MiniAudioBackend)
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
    # wzAudio removed — Story 5.2.1: BGM replaced by miniaudio via g_platformAudio (MiniAudioBackend)
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
