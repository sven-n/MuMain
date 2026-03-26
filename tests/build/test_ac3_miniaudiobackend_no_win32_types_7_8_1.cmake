# Flow Code: VS0-QUAL-BUILD-AUDIO
# Story 7.8.1: Audio Interface Win32 Type Cleanup [VS0-QUAL-BUILD-AUDIO]
# AC-3: Platform/MiniAudio/MiniAudioBackend.h — override declarations must match
#        the updated IPlatformAudio.h interface exactly — no Win32 types remain.
#
# RED PHASE: Fails until Task 3 in story 7.8.1 updates override signatures to match
#            the portable interface. As long as HRESULT/BOOL/OBJECT* are present in
#            the override declarations, this test fails.
#
# GREEN PHASE: MiniAudioBackend.h override signatures exactly match the updated
#              IPlatformAudio.h interface (bool return, bool params, void* or absent).

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED MINIAUDIO_BACKEND_H)
    message(FATAL_ERROR "AC-3: MINIAUDIO_BACKEND_H variable not defined")
endif()

if(NOT EXISTS "${MINIAUDIO_BACKEND_H}")
    message(FATAL_ERROR "AC-3: MiniAudioBackend.h not found at: ${MINIAUDIO_BACKEND_H}")
endif()

file(READ "${MINIAUDIO_BACKEND_H}" content)

# ---------------------------------------------------------------------------
# Check 1: HRESULT must NOT appear in override declarations
# In RED phase: "HRESULT PlaySound" override is present → FAIL
# In GREEN phase: replaced with "bool PlaySound" → PASS
# ---------------------------------------------------------------------------
string(FIND "${content}" "HRESULT PlaySound" pos_hresult)
if(NOT pos_hresult EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAILED: 'HRESULT PlaySound' found in MiniAudioBackend.h override declarations\n"
        "Fix: Update PlaySound() override to return bool, matching updated IPlatformAudio.h")
endif()

# ---------------------------------------------------------------------------
# Check 2: BOOL parameters must NOT appear in override declarations
# ---------------------------------------------------------------------------
string(FIND "${content}" "BOOL looped" pos_bool_looped)
if(NOT pos_bool_looped EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAILED: 'BOOL looped' parameter found in MiniAudioBackend.h\n"
        "Fix: Update PlaySound() override parameter to bool looped")
endif()

string(FIND "${content}" "BOOL resetPosition" pos_bool_reset)
if(NOT pos_bool_reset EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAILED: 'BOOL resetPosition' parameter found in MiniAudioBackend.h\n"
        "Fix: Update StopSound() override parameter to bool resetPosition")
endif()

string(FIND "${content}" "BOOL enforce" pos_bool_enforce)
if(NOT pos_bool_enforce EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAILED: 'BOOL enforce' parameter found in MiniAudioBackend.h\n"
        "Fix: Update PlayMusic()/StopMusic() override parameters to bool enforce")
endif()

# ---------------------------------------------------------------------------
# Check 3: OBJECT* must NOT appear in override method signatures
# Note: m_soundObjects array using OBJECT* is also checked here as it blocks
# compilation on non-Windows when OBJECT is not fully defined.
# In RED phase: "OBJECT* pObject" in PlaySound and "const OBJECT*" in m_soundObjects → FAIL
# In GREEN phase: void* or absent → PASS
# ---------------------------------------------------------------------------
string(FIND "${content}" "OBJECT* pObject" pos_object_param)
if(NOT pos_object_param EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAILED: 'OBJECT* pObject' found in MiniAudioBackend.h override declaration\n"
        "Fix: Update PlaySound() override to use void* pObject or remove the parameter")
endif()

string(FIND "${content}" "const OBJECT*" pos_const_object)
if(NOT pos_const_object EQUAL -1)
    message(FATAL_ERROR
        "AC-3 FAILED: 'const OBJECT*' found in MiniAudioBackend.h (likely m_soundObjects array)\n"
        "Fix: Replace 'const OBJECT*' with 'const void*' for m_soundObjects member array")
endif()

# ---------------------------------------------------------------------------
# Verify file exists and is non-trivial (regression guard)
# ---------------------------------------------------------------------------
string(LENGTH "${content}" content_len)
if(content_len LESS 100)
    message(FATAL_ERROR "AC-3: MiniAudioBackend.h appears empty or too short (${content_len} bytes)")
endif()

message(STATUS "AC-3 PASSED: MiniAudioBackend.h override declarations have no Win32 types (no HRESULT, BOOL, OBJECT*)")
