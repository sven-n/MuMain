# Story 7.6.1: AC-11 — MinGW CI build continues to pass (no regression)
# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
#
# RED PHASE:  Test FAILS if story 7.6.1 changes introduce new #ifdef _WIN32 guards
#             in game logic files (non-platform-abstraction code).
#             Project rule: no #ifdef _WIN32 in game logic — only in PlatformCompat.h.
#
# GREEN PHASE: Test PASSES when:
#   - Modified source files in game logic directories have no new Win32 guards
#   - All cross-platform fixes use PlatformCompat.h stubs or CMake conditionals
#   - PlatformCompat.h itself is the only place allowed to have Win32 guards
#
# Validates:
#   AC-11 — No regression in Windows MinGW build path; game logic files stay platform-neutral

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED MUMAIN_SOURCE_DIR)
    message(FATAL_ERROR "MUMAIN_SOURCE_DIR must be set to the MuMain/src/source directory")
endif()

# Files modified by story 7.6.1 that must not have new #ifdef _WIN32 guards in game logic
# Note: DSwaveIO.h is a platform header — it IS allowed to have #ifdef _WIN32
# Note: PlatformCompat.h is the platform abstraction — it IS allowed to have #ifdef _WIN32
# Note: CMakeLists.txt is not a source file — CMake conditionals are correct approach
# The game logic files to check are the cross-platform ones:
set(GAME_LOGIC_FILES
    "${MUMAIN_SOURCE_DIR}/ThirdParty/xstreambuf.cpp"
    "${MUMAIN_SOURCE_DIR}/Platform/posix/PosixSignalHandlers.cpp"
    "${MUMAIN_SOURCE_DIR}/Data/ZzzOpenData.cpp"
)

set(found_violations FALSE)

foreach(source_file IN LISTS GAME_LOGIC_FILES)
    if(NOT EXISTS "${source_file}")
        message(WARNING "AC-11 SKIP: ${source_file} not found — skipping regression check")
        continue()
    endif()

    file(READ "${source_file}" source_content)
    get_filename_component(filename "${source_file}" NAME)

    # Count #ifdef _WIN32 occurrences
    string(REGEX MATCHALL "#ifdef _WIN32" win32_guards "${source_content}")
    list(LENGTH win32_guards guard_count)

    # xstreambuf.cpp: third-party file, should have NO #ifdef _WIN32 guards
    # (the fix is a C++ type cast, not a platform guard)
    if(filename STREQUAL "xstreambuf.cpp")
        if(guard_count GREATER 0)
            message(WARNING
                "AC-11 WARN: xstreambuf.cpp has ${guard_count} #ifdef _WIN32 guard(s).\n"
                "Story 7.6.1 fix (delete[] static_cast<char*>) should not require Win32 guards.\n"
                "ThirdParty/ is excluded from lint, but game logic patterns still apply here.")
            set(found_violations TRUE)
        endif()
    endif()

    # PosixSignalHandlers.cpp: POSIX-only file, should have NO #ifdef _WIN32 guards
    if(filename STREQUAL "PosixSignalHandlers.cpp")
        if(guard_count GREATER 0)
            message(WARNING
                "AC-11 WARN: PosixSignalHandlers.cpp has ${guard_count} #ifdef _WIN32 guard(s).\n"
                "This is a POSIX-only file — SA_SIGINFO fix should not need Win32 guards.")
            set(found_violations TRUE)
        endif()
    endif()

    # ZzzOpenData.cpp: game logic file — __has_warning pragma is Clang-specific but portable
    # (uses #if defined(__has_warning) which is a compiler feature check, not Win32)
    if(filename STREQUAL "ZzzOpenData.cpp")
        if(guard_count GREATER 1)
            message(WARNING
                "AC-11 WARN: ZzzOpenData.cpp has ${guard_count} #ifdef _WIN32 guards.\n"
                "The __has_warning pragma fix must not use Win32 guards — use defined(__has_warning).")
            set(found_violations TRUE)
        endif()
    endif()
endforeach()

if(found_violations)
    message(FATAL_ERROR
        "AC-11 FAIL: New #ifdef _WIN32 guards found in story 7.6.1 modified game logic files.\n"
        "All 7.6.1 fixes must use platform-neutral patterns:\n"
        "  - C++ typed casts (static_cast) instead of Win32 guards\n"
        "  - PlatformCompat.h stubs for Win32 API symbols\n"
        "  - CMake conditionals for file exclusion\n"
        "  - __has_warning() feature checks for compiler-specific pragmas")
endif()

message(STATUS "AC-11 PASS: No new Win32 guards in story 7.6.1 modified game logic files — MinGW regression guard passed")
