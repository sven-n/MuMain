# Flow Code: VS0-QUAL-WIN32CLEAN-STRINCLUDE
# Story 7.6.2: Win32 String Conversion and Include Guard Cleanup
# AC-4: Data/GlobalBitmap.cpp uses mu_wchar_to_utf8() instead of WideCharToMultiByte;
#        #ifdef _WIN32 block removed.
#
# RED PHASE: Fails until Task 3 (GlobalBitmap.cpp) is complete.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "AC-4: SOURCE_FILE variable not defined")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-4: GlobalBitmap.cpp not found at: ${SOURCE_FILE}")
endif()

file(READ "${SOURCE_FILE}" content)

# Check 1: WideCharToMultiByte must NOT appear
string(FIND "${content}" "WideCharToMultiByte" _pos_wide)
if(NOT _pos_wide EQUAL -1)
    message(FATAL_ERROR "AC-4 FAILED: GlobalBitmap.cpp still uses WideCharToMultiByte — replace with mu_wchar_to_utf8()")
endif()

# Check 2: mu_wchar_to_utf8 MUST appear
string(FIND "${content}" "mu_wchar_to_utf8" _pos_mu)
if(_pos_mu EQUAL -1)
    message(FATAL_ERROR "AC-4 FAILED: GlobalBitmap.cpp does not use mu_wchar_to_utf8() — migration not complete")
endif()

# Check 3: No #ifdef _WIN32 wrapping WideCharToMultiByte call site
string(REGEX MATCH "#ifdef _WIN32[^\n]*\n[^\n]*WideCharToMultiByte" _match "${content}")
if(_match)
    message(FATAL_ERROR "AC-4 FAILED: GlobalBitmap.cpp still has #ifdef _WIN32 wrapping WideCharToMultiByte")
endif()

message(STATUS "AC-4 PASSED: GlobalBitmap.cpp uses mu_wchar_to_utf8() and has no WideCharToMultiByte")
