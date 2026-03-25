# Flow Code: VS0-QUAL-WIN32CLEAN-STRINCLUDE
# Story 7.6.2: Win32 String Conversion and Include Guard Cleanup
# AC-3: Core/StringUtils.h uses mu_wchar_to_utf8() instead of WideCharToMultiByte;
#        no bare #ifdef _WIN32 wrapping any utility function.
#
# RED PHASE: Fails until Task 2 (StringUtils.h) is complete.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "AC-3: SOURCE_FILE variable not defined")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-3: StringUtils.h not found at: ${SOURCE_FILE}")
endif()

file(READ "${SOURCE_FILE}" content)

# Check 1: WideCharToMultiByte must NOT appear
string(FIND "${content}" "WideCharToMultiByte" _pos_wide)
if(NOT _pos_wide EQUAL -1)
    message(FATAL_ERROR "AC-3 FAILED: StringUtils.h still uses WideCharToMultiByte — replace with mu_wchar_to_utf8()")
endif()

# Check 2: mu_wchar_to_utf8 MUST appear
string(FIND "${content}" "mu_wchar_to_utf8" _pos_mu)
if(_pos_mu EQUAL -1)
    message(FATAL_ERROR "AC-3 FAILED: StringUtils.h does not use mu_wchar_to_utf8() — migration not complete")
endif()

# Check 3: No bare #ifdef _WIN32 wrapping a function body or utility
# Bare #ifdef _WIN32 around includes without #else is the anti-pattern.
# An include-selection guard WITH a proper #else is acceptable.
# We check for the specific anti-pattern: #ifdef _WIN32 wrapping just WideCharToMultiByte calls.
string(REGEX MATCH "#ifdef _WIN32[^\n]*\n[^\n]*WideCharToMultiByte" _match_win32_wide "${content}")
if(_match_win32_wide)
    message(FATAL_ERROR "AC-3 FAILED: StringUtils.h still has #ifdef _WIN32 guard wrapping WideCharToMultiByte call")
endif()

message(STATUS "AC-3 PASSED: StringUtils.h uses mu_wchar_to_utf8() and has no WideCharToMultiByte")
