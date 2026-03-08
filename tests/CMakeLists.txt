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

# Story 3.1.2: Connection.h Cross-Platform Updates [VS1-NET-CONNECTION-XPLAT]
# Tests mu::platform::Load/GetSymbol graceful failure paths used by Connection.h.
# No actual .NET library required at test time.
target_sources(MuTests PRIVATE platform/test_connection_library_load.cpp)

# Story 3.4.1: Connection Error Messaging & Graceful Degradation [VS1-NET-ERROR-MESSAGING]
# RED PHASE: Tests FormatLibraryNotFoundMessage() and FormatSymbolNotFoundMessage() helpers
# extracted from Connection.cpp. Tests compile but FAIL until Connection.cpp exposes these
# free functions in the DotNetBridge namespace. No .NET library required at test time.
target_sources(MuTests PRIVATE network/test_connection_error_messages.cpp)

# Story 3.2.1: char16_t Encoding at .NET Interop Boundary [VS1-NET-CHAR16T-ENCODING]
# RED PHASE: Tests mu_wchar_to_char16 / mu_char16_to_wchar round-trips and byte-level
# UTF-16LE baseline for Korean, Latin, mixed-script, null, and IP address strings.
# Tests fail until PlatformCompat.h has the mu_wchar_to_char16 / mu_char16_to_wchar utilities.
target_sources(MuTests PRIVATE platform/test_char16t_encoding.cpp)

# Stories 3.3.1 / 3.3.2: Platform connectivity smoke tests
# MU_TEST_LIBRARY_PATH is set to the absolute path of the platform's .NET AOT library.
# Using a single if/elseif block avoids potential duplicate-definition warnings if
# a future platform satisfies both conditions. (Story 3.3.2 fix L-1)
if(APPLE AND EXISTS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MUnique.Client.Library.dylib")
    # Story 3.3.1: macOS [VS1-NET-VALIDATE-MACOS]
    target_compile_definitions(MuTests PRIVATE
        MU_TEST_LIBRARY_PATH="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MUnique.Client.Library.dylib"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND EXISTS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MUnique.Client.Library.so")
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
