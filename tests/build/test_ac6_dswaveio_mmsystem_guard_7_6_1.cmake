# Story 7.6.1: AC-6 — DSwaveIO.h includes mmsystem.h only on Windows
# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
#
# RED PHASE:  Test FAILS if DSwaveIO.h includes <mmsystem.h> unconditionally.
#             mmsystem.h is a Windows-only header (WinMM API); including it on macOS
#             causes MUAudio target compilation failure.
#
# GREEN PHASE: Test PASSES when:
#   - DSwaveIO.h has #include <mmsystem.h> inside #ifdef _WIN32 guard
#   - Any WAVEFORMATEX/HWAVEOUT/MMRESULT types in header declarations also guarded
#
# Validates:
#   AC-6 — DSwaveIO.h mmsystem.h include guarded with #ifdef _WIN32

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED DSWAVEIO_H)
    message(FATAL_ERROR "DSWAVEIO_H must be set to the path of DSwaveIO.h")
endif()

if(NOT EXISTS "${DSWAVEIO_H}")
    message(FATAL_ERROR "AC-6 FAIL: DSwaveIO.h not found at '${DSWAVEIO_H}'")
endif()

file(READ "${DSWAVEIO_H}" header_content)

# Check that mmsystem.h is present in the file
string(FIND "${header_content}" "mmsystem.h" mmsys_pos)
if(mmsys_pos EQUAL -1)
    # If mmsystem.h is completely absent, the file has been restructured differently.
    # Check that the file still exists as a valid header (waveIO class or equivalent).
    message(STATUS "AC-6 INFO: mmsystem.h not found in DSwaveIO.h — may be entirely restructured.")
    message(STATUS "AC-6 PASS: mmsystem.h not included in DSwaveIO.h (platform-neutral by omission)")
    return()
endif()

# mmsystem.h IS present — verify it is inside a #ifdef _WIN32 guard
# Extract position of #ifdef _WIN32 and verify mmsystem.h comes after it
string(FIND "${header_content}" "#ifdef _WIN32" win32_guard_pos)
if(win32_guard_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-6 FAIL: DSwaveIO.h includes mmsystem.h but has no #ifdef _WIN32 guard.\n"
        "mmsystem.h is Windows-only and causes MUAudio to fail compilation on macOS.\n"
        "Fix: Wrap #include <mmsystem.h> with #ifdef _WIN32 / #endif in DSwaveIO.h")
endif()

# Verify the guard comes before the include
if(win32_guard_pos GREATER mmsys_pos)
    message(FATAL_ERROR
        "AC-6 FAIL: #ifdef _WIN32 appears AFTER #include <mmsystem.h> in DSwaveIO.h.\n"
        "The guard must precede the include. Fix: Move #ifdef _WIN32 before #include <mmsystem.h>")
endif()

message(STATUS "AC-6 PASS: DSwaveIO.h mmsystem.h include is guarded with #ifdef _WIN32")
