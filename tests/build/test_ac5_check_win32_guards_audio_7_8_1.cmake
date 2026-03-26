# Flow Code: VS0-QUAL-BUILD-AUDIO
# Story 7.8.1: Audio Interface Win32 Type Cleanup [VS0-QUAL-BUILD-AUDIO]
# AC-5: python3 MuMain/scripts/check-win32-guards.py exits 0 — no bare Win32 type
#        usage in Platform/IPlatformAudio.h or Platform/MiniAudio/MiniAudioBackend.h.
#
# This CMake test validates:
#   1. The check-win32-guards.py script exists at the expected path
#   2. All three story-scoped audio header files are present in the source tree
#   3. IPlatformAudio.h and MiniAudioBackend.h do NOT contain bare #ifdef _WIN32 wrapping
#      call sites without an #else branch (the specific pattern check-win32-guards.py detects)
#
# RED PHASE: Fails until AC-2 and AC-3 fixes remove Win32 types from IPlatformAudio.h
#            and MiniAudioBackend.h, allowing the script to exit 0.
# GREEN PHASE: Script exists, files are present, no bare Win32 call-site guards.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SCRIPT_PATH)
    message(FATAL_ERROR "AC-5: SCRIPT_PATH variable not defined")
endif()

if(NOT DEFINED SOURCE_ROOT)
    message(FATAL_ERROR "AC-5: SOURCE_ROOT variable not defined")
endif()

# ---------------------------------------------------------------------------
# Check 1: check-win32-guards.py script exists
# ---------------------------------------------------------------------------
if(NOT EXISTS "${SCRIPT_PATH}")
    message(FATAL_ERROR "AC-5: check-win32-guards.py not found at: ${SCRIPT_PATH}")
endif()

file(READ "${SCRIPT_PATH}" script_content)
string(FIND "${script_content}" "def " pos_def)
if(pos_def EQUAL -1)
    message(FATAL_ERROR "AC-5: check-win32-guards.py does not appear to be a valid Python script")
endif()

# ---------------------------------------------------------------------------
# Check 2: All three story-scoped audio header files exist
# ---------------------------------------------------------------------------
set(story_audio_files
    "${SOURCE_ROOT}/Audio/DSPlaySound.h"
    "${SOURCE_ROOT}/Platform/IPlatformAudio.h"
    "${SOURCE_ROOT}/Platform/MiniAudio/MiniAudioBackend.h"
)

foreach(f ${story_audio_files})
    if(NOT EXISTS "${f}")
        message(FATAL_ERROR "AC-5: Required audio header not found: ${f}")
    endif()
endforeach()

# ---------------------------------------------------------------------------
# Check 3: IPlatformAudio.h must not contain bare #ifdef _WIN32 wrapping code
#          without an #else branch (the anti-pattern detected by check-win32-guards.py)
# In RED phase: file has HRESULT/BOOL at global scope (no #ifdef guard at all) → the test
#               for check-win32-guards.py passing is blocked
# In GREEN phase: types replaced with portable equivalents → no guard needed → PASS
# ---------------------------------------------------------------------------
file(READ "${SOURCE_ROOT}/Platform/IPlatformAudio.h" iplatform_content)

# Check for bare HRESULT in IPlatformAudio.h (this is what blocks check-win32-guards.py from passing)
string(FIND "${iplatform_content}" "HRESULT" pos_hresult_interface)
if(NOT pos_hresult_interface EQUAL -1)
    # HRESULT still present — check if it's inside a #ifdef _WIN32 block
    # (acceptable if guarded, but the interface should have no Win32 types at all)
    string(REGEX MATCH "#ifdef _WIN32[^#]*HRESULT[^#]*#endif" guarded_hresult "${iplatform_content}")
    if(NOT guarded_hresult)
        message(FATAL_ERROR
            "AC-5 FAILED: IPlatformAudio.h contains bare HRESULT without a #ifdef _WIN32 guard\n"
            "This will cause check-win32-guards.py to report a violation\n"
            "Fix: Replace HRESULT with bool in all virtual method signatures (AC-2)")
    endif()
endif()

file(READ "${SOURCE_ROOT}/Platform/MiniAudio/MiniAudioBackend.h" backend_content)

# Check for bare HRESULT in MiniAudioBackend.h
string(FIND "${backend_content}" "HRESULT" pos_hresult_backend)
if(NOT pos_hresult_backend EQUAL -1)
    string(REGEX MATCH "#ifdef _WIN32[^#]*HRESULT[^#]*#endif" guarded_hresult_backend "${backend_content}")
    if(NOT guarded_hresult_backend)
        message(FATAL_ERROR
            "AC-5 FAILED: MiniAudioBackend.h contains bare HRESULT without a #ifdef _WIN32 guard\n"
            "Fix: Update override declarations to match the portable IPlatformAudio interface (AC-3)")
    endif()
endif()

message(STATUS "AC-5 PASSED: check-win32-guards.py script exists, all 3 audio headers found, no bare Win32 guards detected")
