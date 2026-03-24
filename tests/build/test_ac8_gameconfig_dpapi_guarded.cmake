# Story 7.3.0: AC-8 — GameConfig.cpp DPAPI calls guarded by #ifdef _WIN32
# Flow Code: VS0-QUAL-BUILDCOMPAT-MACOS
#
# RED PHASE:  Test FAILS before story 7.3.0 is implemented.
#             DATA_BLOB, CryptProtectData, and CryptUnprotectData are used unguarded in
#             GameConfig.cpp, causing "undeclared identifier" errors on macOS/Linux.
#
# GREEN PHASE: Test PASSES after fix.
#             All DPAPI identifiers appear only inside #ifdef _WIN32 blocks.
#             Non-Windows path has stub functions returning input unchanged.
#
# Validates:
#   AC-8 — DATA_BLOB/CryptProtectData/CryptUnprotectData are inside #ifdef _WIN32 guard

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "SOURCE_FILE must be set to the path of GameConfig.cpp")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-8 FAIL: GameConfig.cpp not found at '${SOURCE_FILE}'")
endif()

file(READ "${SOURCE_FILE}" source_content)

# ---------------------------------------------------------------------------
# Strategy: Extract the cross-platform section (before the first #ifdef _WIN32)
# and verify none of the DPAPI identifiers appear there.
# ---------------------------------------------------------------------------

string(FIND "${source_content}" "#ifdef _WIN32" guard_pos)
if(guard_pos EQUAL -1)
    # No #ifdef _WIN32 guard at all — entire file is cross-platform section
    set(cross_platform_section "${source_content}")
    set(has_guard FALSE)
else()
    string(SUBSTRING "${source_content}" 0 ${guard_pos} cross_platform_section)
    set(has_guard TRUE)
endif()

set(found_violations FALSE)
set(DPAPI_PATTERNS "DATA_BLOB" "CryptProtectData" "CryptUnprotectData" "dpapi.h" "wincrypt.h")

foreach(pattern IN LISTS DPAPI_PATTERNS)
    string(FIND "${cross_platform_section}" "${pattern}" pattern_pos)
    if(NOT pattern_pos EQUAL -1)
        message(WARNING "AC-8 FAIL: '${pattern}' found in GameConfig.cpp cross-platform section (before #ifdef _WIN32)")
        set(found_violations TRUE)
    endif()
endforeach()

if(found_violations)
    message(FATAL_ERROR
        "AC-8 FAIL: DPAPI identifiers found outside #ifdef _WIN32 in GameConfig.cpp.\n"
        "Fix: wrap DATA_BLOB, CryptProtectData, CryptUnprotectData usage in #ifdef _WIN32.\n"
        "Add #else stubs: DecryptSetting and EncryptSetting return input as-is on non-Windows.")
endif()

# Verify guard exists (DPAPI should be present, just guarded)
if(NOT has_guard)
    message(FATAL_ERROR
        "AC-8 FAIL: No #ifdef _WIN32 guard found in GameConfig.cpp.\n"
        "Expected DPAPI code to be conditionally compiled with #ifdef _WIN32 / #else stub.")
endif()

# Verify DPAPI identifiers exist somewhere (inside the guard)
set(found_dpapi FALSE)
foreach(pattern IN LISTS DPAPI_PATTERNS)
    string(FIND "${source_content}" "${pattern}" pattern_pos)
    if(NOT pattern_pos EQUAL -1)
        set(found_dpapi TRUE)
        break()
    endif()
endforeach()

if(NOT found_dpapi)
    message(WARNING "AC-8 NOTE: No DPAPI identifiers found at all in GameConfig.cpp — verify the #ifdef _WIN32 block exists with DPAPI code")
endif()

message(STATUS "AC-8 PASS: DPAPI identifiers (DATA_BLOB/CryptProtectData/CryptUnprotectData) are inside #ifdef _WIN32 guard in GameConfig.cpp")
