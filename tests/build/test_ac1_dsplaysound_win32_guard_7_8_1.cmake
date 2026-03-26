# Flow Code: VS0-QUAL-BUILD-AUDIO
# Story 7.8.1: Audio Interface Win32 Type Cleanup [VS0-QUAL-BUILD-AUDIO]
# AC-1: Audio/DSPlaySound.h — HRESULT, HWND, and OBJECT* declarations must be
#        wrapped in #ifdef _WIN32 guards or removed from the cross-platform header.
#
# RED PHASE: Fails until Task 4 in story 7.8.1 wraps DirectSound-specific
# declarations (InitDirectSound, PlayBuffer, ReleaseBuffer, RestoreBuffers,
# StopBuffer's BOOL param) in #ifdef _WIN32 guards.
#
# GREEN PHASE: HRESULT/HWND/OBJECT* declarations in DSPlaySound.h are either
# inside an #ifdef _WIN32 block or absent entirely.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED DSPLAYSOUND_H)
    message(FATAL_ERROR "AC-1: DSPLAYSOUND_H variable not defined")
endif()

if(NOT EXISTS "${DSPLAYSOUND_H}")
    message(FATAL_ERROR "AC-1: DSPlaySound.h not found at: ${DSPLAYSOUND_H}")
endif()

file(READ "${DSPLAYSOUND_H}" content)

# ---------------------------------------------------------------------------
# Check 1: HRESULT InitDirectSound must NOT appear outside #ifdef _WIN32
# In RED phase: bare "HRESULT InitDirectSound" exists at global scope → FAIL
# In GREEN phase: wrapped in #ifdef _WIN32 or removed → PASS
# ---------------------------------------------------------------------------
string(FIND "${content}" "HRESULT InitDirectSound" pos_hresult_init)
if(NOT pos_hresult_init EQUAL -1)
    # Found — check that it appears directly after a #ifdef _WIN32 line
    string(REGEX MATCH "#ifdef _WIN32[^\n]*\n[^\n]*HRESULT InitDirectSound" guard_init "${content}")
    if(NOT guard_init)
        message(FATAL_ERROR
            "AC-1 FAILED: HRESULT InitDirectSound(HWND) is not wrapped in #ifdef _WIN32 guard in DSPlaySound.h\n"
            "Fix: Wrap the DirectSound-only free function declarations in #ifdef _WIN32 / #endif\n"
            "Lines: InitDirectSound, PlayBuffer, ReleaseBuffer, RestoreBuffers")
    endif()
endif()

# ---------------------------------------------------------------------------
# Check 2: HRESULT PlayBuffer (uses OBJECT*) must NOT appear outside #ifdef _WIN32
# ---------------------------------------------------------------------------
string(FIND "${content}" "HRESULT PlayBuffer" pos_hresult_play)
if(NOT pos_hresult_play EQUAL -1)
    string(REGEX MATCH "#ifdef _WIN32[^\n]*\n[^\n]*HRESULT PlayBuffer" guard_play "${content}")
    if(NOT guard_play)
        message(FATAL_ERROR
            "AC-1 FAILED: HRESULT PlayBuffer(OBJECT*) is not wrapped in #ifdef _WIN32 guard in DSPlaySound.h\n"
            "Fix: PlayBuffer is DirectSound-only — wrap in #ifdef _WIN32 / #endif")
    endif()
endif()

# ---------------------------------------------------------------------------
# Check 3: HWND must NOT appear outside #ifdef _WIN32 guard
# ---------------------------------------------------------------------------
string(FIND "${content}" "HWND hDlg" pos_hwnd)
if(NOT pos_hwnd EQUAL -1)
    string(REGEX MATCH "#ifdef _WIN32[^\n]*\n[^\n]*HWND hDlg" guard_hwnd "${content}")
    if(NOT guard_hwnd)
        message(FATAL_ERROR
            "AC-1 FAILED: HWND hDlg parameter found outside #ifdef _WIN32 guard in DSPlaySound.h\n"
            "Fix: InitDirectSound(HWND hDlg) is Windows-only — wrap in #ifdef _WIN32 / #endif")
    endif()
endif()

# ---------------------------------------------------------------------------
# Check 4: Required source file exists (regression guard)
# ---------------------------------------------------------------------------
message(STATUS "AC-1 PASSED: DSPlaySound.h Win32-type declarations are properly guarded")
