# Flow Code: VS0-QUAL-WIN32CLEAN-STRINCLUDE
# Story 7.6.2: Win32 String Conversion and Include Guard Cleanup
# AC-9: Data/Items/ItemStructs.h, Data/Skills/SkillStructs.h —
#        windows.h include-selection guards have complete #else → PlatformCompat.h branch.
#
# RED PHASE: Fails until Task 8 (Data struct headers) is complete.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_ROOT)
    message(FATAL_ERROR "AC-9: SOURCE_ROOT variable not defined")
endif()

set(data_headers
    "${SOURCE_ROOT}/Data/Items/ItemStructs.h"
    "${SOURCE_ROOT}/Data/Skills/SkillStructs.h"
)

foreach(header_file ${data_headers})
    if(NOT EXISTS "${header_file}")
        message(FATAL_ERROR "AC-9 FAILED: Data struct header not found: ${header_file}")
    endif()

    file(READ "${header_file}" content)
    get_filename_component(header_name "${header_file}" NAME)

    # Check 1: If file has windows.h, it MUST have an #else branch
    string(FIND "${content}" "windows.h" _pos_windows_h)
    if(NOT _pos_windows_h EQUAL -1)
        string(FIND "${content}" "#else" _pos_else)
        if(_pos_else EQUAL -1)
            message(FATAL_ERROR "AC-9 FAILED: ${header_name} includes windows.h but has no #else branch — add #else / #include \"PlatformCompat.h\"")
        endif()

        # Verify #else branch references PlatformCompat.h
        string(REGEX MATCH "#else[^\n]*\n[^\n]*PlatformCompat" _match_else_compat "${content}")
        if(NOT _match_else_compat)
            message(FATAL_ERROR "AC-9 FAILED: ${header_name} has #else branch but does not include PlatformCompat.h")
        endif()
    endif()

    # Check 2: If the file has #ifdef _WIN32 at all, each one must have a #else
    # Count #ifdef _WIN32 and #else occurrences — they should match for include guards
    string(REGEX MATCHALL "#ifdef _WIN32" _all_guards "${content}")
    list(LENGTH _all_guards _guard_count)
    string(REGEX MATCHALL "#else" _all_else "${content}")
    list(LENGTH _all_else _else_count)

    if(_guard_count GREATER 0 AND _else_count EQUAL 0)
        message(FATAL_ERROR "AC-9 FAILED: ${header_name} has ${_guard_count} #ifdef _WIN32 guards but zero #else branches")
    endif()

    message(STATUS "AC-9 PASSED: ${header_name} has correct include-selection with #else / PlatformCompat.h (guards: ${_guard_count}, else: ${_else_count})")
endforeach()

message(STATUS "AC-9 PASSED: ItemStructs.h and SkillStructs.h have correct windows.h include-selection with #else branch")
