include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(MuTests
    core/test_timer.cpp
    platform/test_platform_types.cpp
    platform/test_platform_compat.cpp
    platform/test_platform_keys.cpp
    platform/test_platform_library.cpp
    platform/test_platform_window.cpp
)

# Story 2.2.1: SDL3 Keyboard Input Migration [VS1-SDL-INPUT-KEYBOARD]
# test_platform_input.cpp is guarded by #ifdef MU_ENABLE_SDL3 internally,
# but we add it to MuTests unconditionally so the translation unit is always
# compiled. The #ifdef inside the file ensures SDL3-specific code is excluded
# when MU_ENABLE_SDL3 is OFF (CI / MinGW Strategy B).
target_sources(MuTests PRIVATE platform/test_platform_input.cpp)

# Story 2.2.2: SDL3 Mouse Input Migration [VS1-SDL-INPUT-MOUSE]
# test_platform_mouse.cpp is guarded by #ifdef MU_ENABLE_SDL3 internally.
# Added unconditionally so the TU is always compiled; code is excluded when
# MU_ENABLE_SDL3 is OFF (CI / MinGW Strategy B).
target_sources(MuTests PRIVATE platform/test_platform_mouse.cpp)

# Story 2.2.3: SDL3 Text Input Migration [VS1-SDL-INPUT-TEXT]
# test_platform_text_input.cpp is guarded by #ifdef MU_ENABLE_SDL3 internally.
# Added unconditionally so the TU is always compiled; code is excluded when
# MU_ENABLE_SDL3 is OFF (CI / MinGW Strategy B).
target_sources(MuTests PRIVATE platform/test_platform_text_input.cpp)

# Story 7.1.1: Cross-Platform Error Reporting [VS0-QUAL-ERRORREPORT-XPLAT]
# RED PHASE: test_error_report.cpp exercises post-refactor API (std::ofstream/std::filesystem).
# Tests fail until ErrorReport.cpp is refactored in story 7.1.1.
target_sources(MuTests PRIVATE core/test_error_report.cpp)

# Story 7.2.1: Frame Time Instrumentation [VS0-QUAL-FRAMETIMER]
# GREEN PHASE: mu::MuTimer is implemented in MuTimer.h and MuTimer.cpp. All 6 tests pass.
target_sources(MuTests PRIVATE core/test_mu_timer.cpp)

# Story 4.1.1: Ground Truth Capture Mechanism [VS1-RENDER-GROUNDTRUTH-CAPTURE]
# GREEN PHASE: GroundTruthCapture.h/cpp implemented. All SSIM tests pass.
# ComputeSSIM() is always compiled (no ENABLE_GROUND_TRUTH_CAPTURE guard).
# glReadPixels-dependent code is guarded by #ifdef ENABLE_GROUND_TRUTH_CAPTURE in
# GroundTruthCapture.cpp; the glReadPixels stub in stdafx.h ensures compilation on all platforms.
target_sources(MuTests PRIVATE core/test_ground_truth.cpp)

# Story 3.1.2: Connection.h Cross-Platform Updates [VS1-NET-CONNECTION-XPLAT]
# Tests mu::platform::Load/GetSymbol graceful failure paths used by Connection.h.
# No actual .NET library required at test time.
target_sources(MuTests PRIVATE platform/test_connection_library_load.cpp)

# Story 3.4.1: Connection Error Messaging & Graceful Degradation [VS1-NET-ERROR-MESSAGING]
# RED PHASE: Tests FormatLibraryNotFoundMessage() and FormatSymbolNotFoundMessage() helpers
# extracted from Connection.cpp. Tests compile but FAIL until Connection.cpp exposes these
# free functions in the DotNetBridge namespace. No .NET library required at test time.
target_sources(MuTests PRIVATE network/test_connection_error_messages.cpp)

# Story 3.4.2: Server Connection Configuration [VS1-NET-CONFIG-SERVER]
# RED PHASE: Tests ValidateServerPort() and ValidateServerIP() helpers extracted into
# Core/GameConfigValidation.h/.cpp. Tests compile but FAIL until GameConfigValidation.cpp
# is present with the correct implementations. No config.ini required at test time.
target_sources(MuTests PRIVATE network/test_server_config_validation.cpp)

# Story 7.1.2: POSIX Signal Handlers for Crash Diagnostics [VS0-QUAL-SIGNAL-HANDLERS]
# RED PHASE: test_posix_signal_handlers.cpp verifies InstallSignalHandlers() installs
# SA_SIGACTION handlers for SIGSEGV/SIGABRT/SIGBUS. Guarded by #ifndef _WIN32 internally
# so the TU compiles on Windows (MinGW CI) — the test body is a no-op stub on Windows.
target_sources(MuTests PRIVATE platform/test_posix_signal_handlers.cpp)

# Story 4.2.1: MuRenderer Core API with OpenGL Backend [VS1-RENDER-ABSTRACT-CORE]
# RED PHASE: test_murenderer.cpp tests MatrixStack push/pop logic and BlendMode state
# tracking via an inline IMuRenderer test-double (no OpenGL calls in the test TU).
# Tests compile but FAIL until MuRenderer.h, MatrixStack.h, and MatrixStack.cpp are
# implemented in src/source/RenderFX/.
target_sources(MuTests PRIVATE render/test_murenderer.cpp)

# Story 3.2.1: char16_t Encoding at .NET Interop Boundary [VS1-NET-CHAR16T-ENCODING]
# RED PHASE: Tests mu_wchar_to_char16 / mu_char16_to_wchar round-trips and byte-level
# UTF-16LE baseline for Korean, Latin, mixed-script, null, and IP address strings.
# Tests fail until PlatformCompat.h has the mu_wchar_to_char16 / mu_char16_to_wchar utilities.
target_sources(MuTests PRIVATE platform/test_char16t_encoding.cpp)

# Stories 3.3.1 / 3.3.2: Platform connectivity smoke tests
# MU_TEST_LIBRARY_PATH is set to the absolute path of the platform's .NET AOT library.
# Using a single if/elseif block avoids potential duplicate-definition warnings if
# a future platform satisfies both conditions. (Story 3.3.2 fix L-1)
#
# MEDIUM-2 fix (Story 3.3.1 code review): MU_TEST_LIBRARY_PATH is now set unconditionally
# (without EXISTS check) when DOTNETAOT_FOUND and platform match. The previous EXISTS guard
# was evaluated at CMake configure time, but `dotnet publish` runs at build time via
# add_custom_command — so the dylib never exists at configure time on a clean build, causing
# both smoke tests to always SKIP. The test code already handles absent files gracefully
# via std::filesystem::exists() + SKIP macro.
if(APPLE AND DOTNETAOT_FOUND)
    # Story 3.3.1: macOS [VS1-NET-VALIDATE-MACOS]
    target_compile_definitions(MuTests PRIVATE
        MU_TEST_LIBRARY_PATH="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MUnique.Client.Library.dylib"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND DOTNETAOT_FOUND)
    # Story 3.3.2: Linux — MU_TEST_LIBRARY_PATH must be absolute; Linux dlopen() does NOT
    # search the executable directory for bare filenames. (Risk R6 mitigation)
    target_compile_definitions(MuTests PRIVATE
        MU_TEST_LIBRARY_PATH="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MUnique.Client.Library.so"
    )
endif()
# Both test files are always compiled; platform-specific code is guarded by #ifdef internally.
target_sources(MuTests PRIVATE platform/test_macos_connectivity.cpp)
target_sources(MuTests PRIVATE platform/test_linux_connectivity.cpp)

target_link_libraries(MuTests PRIVATE Catch2::Catch2WithMain MUCore MUPlatform)

# Story 2.2.1: Propagate MU_ENABLE_SDL3 to MuTests so test_platform_input.cpp
# #ifdef guards are active when SDL3 is enabled. [VS1-SDL-INPUT-KEYBOARD]
if(MU_ENABLE_SDL3)
    target_compile_definitions(MuTests PRIVATE MU_ENABLE_SDL3)
endif()

include(CTest)
include(Catch)
catch_discover_tests(MuTests)

# Build system validation tests (CMake script mode, no Catch2)
add_subdirectory(build)

# Platform abstraction header tests (Story 1.2.1)
add_subdirectory(platform)
