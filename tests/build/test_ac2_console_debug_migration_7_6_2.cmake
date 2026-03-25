# Flow Code: VS0-QUAL-WIN32CLEAN-STRINCLUDE
# Story 7.6.2: Win32 String Conversion and Include Guard Cleanup
# AC-2: Core/muConsoleDebug.cpp uses mu_wchar_to_utf8() instead of WideCharToMultiByte;
#        <io.h> include-selection block is removed.
#
# RED PHASE: Fails until Task 1 (muConsoleDebug.cpp) is complete.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "AC-2: SOURCE_FILE variable not defined")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-2: muConsoleDebug.cpp not found at: ${SOURCE_FILE}")
endif()

file(READ "${SOURCE_FILE}" content)

# Check 1: WideCharToMultiByte must NOT appear in the file after migration
string(FIND "${content}" "WideCharToMultiByte" _pos_wide)
if(NOT _pos_wide EQUAL -1)
    message(FATAL_ERROR "AC-2 FAILED: muConsoleDebug.cpp still uses WideCharToMultiByte — replace with mu_wchar_to_utf8()")
endif()

# Check 2: mu_wchar_to_utf8 MUST appear in the file
string(FIND "${content}" "mu_wchar_to_utf8" _pos_mu_wchar)
if(_pos_mu_wchar EQUAL -1)
    message(FATAL_ERROR "AC-2 FAILED: muConsoleDebug.cpp does not use mu_wchar_to_utf8() — migration not complete")
endif()

# Check 3: <io.h> bare include must NOT appear (it was Win32 only)
# The correct fix: <unistd.h> is already available via PlatformCompat.h / stdafx.h
string(REGEX MATCH "#include[ \t]*<io\\.h>" _match_io "${content}")
if(_match_io)
    message(FATAL_ERROR "AC-2 FAILED: muConsoleDebug.cpp still includes <io.h> — remove the include-selection block")
endif()

# Check 4: No bare #ifdef _WIN32 wrapping a code block (include-selection pattern is allowed
# only with a proper #else branch — but for <io.h>/<unistd.h> the fix is to remove entirely
# since PlatformCompat.h provides what's needed)
string(REGEX MATCH "#ifdef _WIN32[^\n]*\n[^\n]*io\\.h" _match_win32_io "${content}")
if(_match_win32_io)
    message(FATAL_ERROR "AC-2 FAILED: muConsoleDebug.cpp still has #ifdef _WIN32 guard around <io.h> include")
endif()

message(STATUS "AC-2 PASSED: muConsoleDebug.cpp uses mu_wchar_to_utf8() and has no WideCharToMultiByte or <io.h> include")
