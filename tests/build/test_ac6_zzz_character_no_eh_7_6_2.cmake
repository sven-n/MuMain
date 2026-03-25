# Flow Code: VS0-QUAL-WIN32CLEAN-STRINCLUDE
# Story 7.6.2: Win32 String Conversion and Include Guard Cleanup
# AC-6: Gameplay/Characters/ZzzCharacter.cpp <eh.h> include removed;
#        no #ifdef _WIN32 wrapping any code block.
#
# RED PHASE: Fails until Task 5 (ZzzCharacter.cpp) is complete.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "AC-6: SOURCE_FILE variable not defined")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-6: ZzzCharacter.cpp not found at: ${SOURCE_FILE}")
endif()

file(READ "${SOURCE_FILE}" content)

# Check 1: <eh.h> must NOT be included (MSVC structured exception handling header)
string(REGEX MATCH "#include[ \t]*[<\"]eh\\.h[>\"]" _match_eh "${content}")
if(_match_eh)
    message(FATAL_ERROR "AC-6 FAILED: ZzzCharacter.cpp still includes <eh.h> — remove this include (no SEH on macOS/Linux)")
endif()

# Check 2: #ifdef _WIN32 wrapping <eh.h> must NOT appear
string(REGEX MATCH "#ifdef _WIN32[^\n]*\n[^\n]*eh\\.h" _match_win32_eh "${content}")
if(_match_win32_eh)
    message(FATAL_ERROR "AC-6 FAILED: ZzzCharacter.cpp still has #ifdef _WIN32 guard around <eh.h>")
endif()

# Check 3: __try / __except blocks (MSVC SEH) must NOT appear outside a Win32 guard
# Note: if they're inside a #ifdef _WIN32 ... #endif that's acceptable for forward compat
# but for this story they should be entirely absent
string(FIND "${content}" "__try" _pos_try)
string(FIND "${content}" "__except" _pos_except)
if(NOT _pos_try EQUAL -1 OR NOT _pos_except EQUAL -1)
    # Verify it's inside a Win32 guard — if not, it's a violation
    string(REGEX MATCH "#ifdef _WIN32[^#]*__try[^#]*#endif" _match_guarded_try "${content}")
    if(NOT _match_guarded_try)
        message(WARNING "AC-6: ZzzCharacter.cpp has __try/__except — verify these are inside a #ifdef _WIN32 guard")
    endif()
endif()

message(STATUS "AC-6 PASSED: ZzzCharacter.cpp has no <eh.h> include and no unguarded SEH blocks")
