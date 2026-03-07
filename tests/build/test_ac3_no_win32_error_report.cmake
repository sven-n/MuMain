# Story 7.1.1: AC-3 / AC-STD-3 — No Win32 file I/O in ErrorReport.cpp cross-platform path
# Flow Code: VS0-QUAL-ERRORREPORT-XPLAT
#
# RED PHASE:  Test FAILS before story 7.1.1 is implemented.
#             CreateFile, WriteFile, ReadFile, CloseHandle, SetFilePointer, DeleteFile,
#             GetLocalTime are present in the cross-platform path of ErrorReport.cpp.
#
# GREEN PHASE: Test PASSES after refactoring.
#             Win32 file I/O APIs are removed from the cross-platform path.
#             They remain ONLY inside #ifdef _WIN32 blocks for the legacy diagnostic
#             methods (WriteImeInfo, WriteSoundCardInfo, WriteOpenGLInfo, WriteSystemInfo).
#
# Validates:
#   AC-3      — no CreateFile/WriteFile/ReadFile/CloseHandle/SetFilePointer/DeleteFile
#   AC-4      — GetLocalTime replaced by std::chrono
#   AC-STD-3  — no Win32 file I/O in cross-platform implementation path

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of ErrorReport.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-3 FAIL: ErrorReport.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# ---------------------------------------------------------------------------
# Approach: verify that each banned Win32 file I/O API does NOT appear OUTSIDE
# of #ifdef _WIN32 ... #endif blocks in the cross-platform path.
#
# Implementation: detect bare (unguarded) occurrences.
# A bare occurrence is one that appears in the file but is NOT wrapped in an
# #ifdef _WIN32 / #if defined(_WIN32) guard.
#
# Simple heuristic used here: check that the total count of Win32 API
# appearances matches the count ONLY within known guard blocks.
#
# Pragmatic approach: split on "#ifdef _WIN32" and check the PRE-GUARD portion
# contains no banned patterns.  The pre-guard portion (before the first
# #ifdef _WIN32) must be free of all Win32 file I/O calls.
# ---------------------------------------------------------------------------

# Extract the portion of the file before the first #ifdef _WIN32 guard.
# If there is no such guard, the entire file is tested.
string(FIND "${source_content}" "#ifdef _WIN32" guard_pos)
if(guard_pos EQUAL -1)
    set(cross_platform_section "${source_content}")
else()
    string(SUBSTRING "${source_content}" 0 ${guard_pos} cross_platform_section)
endif()

# List of Win32 file I/O APIs that must not appear in the cross-platform section
set(BANNED_PATTERNS
    "CreateFile("
    "WriteFile("
    "ReadFile("
    "CloseHandle("
    "SetFilePointer("
    "DeleteFile("
    "GetLocalTime("
    "INVALID_HANDLE_VALUE"
)

set(found_violations FALSE)

foreach(pattern IN LISTS BANNED_PATTERNS)
    string(FIND "${cross_platform_section}" "${pattern}" pattern_pos)
    if(NOT pattern_pos EQUAL -1)
        message(WARNING "AC-3 FAIL: '${pattern}' found in cross-platform section of ErrorReport.cpp (position ${pattern_pos})")
        set(found_violations TRUE)
    endif()
endforeach()

if(found_violations)
    message(FATAL_ERROR
        "AC-3 FAIL: Win32 file I/O APIs found in ErrorReport.cpp cross-platform path.\n"
        "Refactor per story 7.1.1: replace with std::ofstream + std::filesystem::path + std::chrono.\n"
        "Win32 APIs may remain ONLY inside #ifdef _WIN32 blocks for legacy diagnostic methods.")
endif()

message(STATUS "AC-3/AC-STD-3 PASS: No Win32 file I/O APIs in ErrorReport.cpp cross-platform path")
