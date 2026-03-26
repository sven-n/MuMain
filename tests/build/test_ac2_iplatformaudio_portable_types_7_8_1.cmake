# Flow Code: VS0-QUAL-BUILD-AUDIO
# Story 7.8.1: Audio Interface Win32 Type Cleanup [VS0-QUAL-BUILD-AUDIO]
# AC-2: Platform/IPlatformAudio.h — pure virtual interface must use only portable C++ types:
#   - HRESULT → bool (return true on success)
#   - BOOL → bool
#   - OBJECT* → removed or replaced with void*
#
# RED PHASE: Fails until Task 2 in story 7.8.1 replaces Win32 types in the interface.
# GREEN PHASE: No bare HRESULT, BOOL, or OBJECT* remain in IPlatformAudio.h method signatures.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED IPLATFORMAUDIO_H)
    message(FATAL_ERROR "AC-2: IPLATFORMAUDIO_H variable not defined")
endif()

if(NOT EXISTS "${IPLATFORMAUDIO_H}")
    message(FATAL_ERROR "AC-2: IPlatformAudio.h not found at: ${IPLATFORMAUDIO_H}")
endif()

file(READ "${IPLATFORMAUDIO_H}" content)

# ---------------------------------------------------------------------------
# Check 1: HRESULT must NOT appear in virtual method signatures
# In RED phase: "virtual HRESULT PlaySound" is present → FAIL
# In GREEN phase: replaced with "virtual bool PlaySound" → PASS
# ---------------------------------------------------------------------------
string(FIND "${content}" "virtual HRESULT" pos_hresult)
if(NOT pos_hresult EQUAL -1)
    message(FATAL_ERROR
        "AC-2 FAILED: 'virtual HRESULT' found in IPlatformAudio.h\n"
        "Fix: Replace HRESULT return types with bool (true = success, false = failure)\n"
        "Affected method: PlaySound()")
endif()

# ---------------------------------------------------------------------------
# Check 2: BOOL parameters must NOT appear in virtual method signatures
# In RED phase: "BOOL looped", "BOOL resetPosition", "BOOL enforce" are present → FAIL
# In GREEN phase: all replaced with bool → PASS
# ---------------------------------------------------------------------------
string(FIND "${content}" "BOOL looped" pos_bool_looped)
if(NOT pos_bool_looped EQUAL -1)
    message(FATAL_ERROR
        "AC-2 FAILED: 'BOOL looped' parameter found in IPlatformAudio.h\n"
        "Fix: Replace BOOL with bool in PlaySound() signature")
endif()

string(FIND "${content}" "BOOL resetPosition" pos_bool_reset)
if(NOT pos_bool_reset EQUAL -1)
    message(FATAL_ERROR
        "AC-2 FAILED: 'BOOL resetPosition' parameter found in IPlatformAudio.h\n"
        "Fix: Replace BOOL with bool in StopSound() signature")
endif()

string(FIND "${content}" "BOOL enforce" pos_bool_enforce)
if(NOT pos_bool_enforce EQUAL -1)
    message(FATAL_ERROR
        "AC-2 FAILED: 'BOOL enforce' parameter found in IPlatformAudio.h\n"
        "Fix: Replace BOOL with bool in PlayMusic() and StopMusic() signatures")
endif()

# ---------------------------------------------------------------------------
# Check 3: OBJECT* must NOT appear in virtual method signatures
# In RED phase: "OBJECT* pObject" is present → FAIL
# In GREEN phase: removed or replaced with void* → PASS
# ---------------------------------------------------------------------------
string(FIND "${content}" "OBJECT* pObject" pos_object)
if(NOT pos_object EQUAL -1)
    message(FATAL_ERROR
        "AC-2 FAILED: 'OBJECT* pObject' found in IPlatformAudio.h\n"
        "Fix: Replace OBJECT* with void* (or remove the parameter if unused by MiniAudio backend)")
endif()

# ---------------------------------------------------------------------------
# Verify file exists and is non-trivial (regression guard)
# ---------------------------------------------------------------------------
string(LENGTH "${content}" content_len)
if(content_len LESS 100)
    message(FATAL_ERROR "AC-2: IPlatformAudio.h appears empty or too short (${content_len} bytes)")
endif()

message(STATUS "AC-2 PASSED: IPlatformAudio.h uses only portable C++ types (no HRESULT, BOOL, OBJECT*)")
