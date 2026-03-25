# Flow Code: VS0-QUAL-WIN32CLEAN-STRINCLUDE
# Story 7.6.2: Win32 String Conversion and Include Guard Cleanup
# AC-8: Scenes/CharacterScene.h, LoginScene.h, MainScene.h, SceneManager.h —
#        windows.h include-selection guards each have a complete #else branch
#        pointing to PlatformCompat.h; no #ifdef _WIN32 wraps any declaration.
#
# RED PHASE: Fails until Task 7 (Scene headers) is complete.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_ROOT)
    message(FATAL_ERROR "AC-8: SOURCE_ROOT variable not defined")
endif()

set(scene_headers
    "${SOURCE_ROOT}/Scenes/CharacterScene.h"
    "${SOURCE_ROOT}/Scenes/LoginScene.h"
    "${SOURCE_ROOT}/Scenes/MainScene.h"
    "${SOURCE_ROOT}/Scenes/SceneManager.h"
)

foreach(header_file ${scene_headers})
    if(NOT EXISTS "${header_file}")
        message(FATAL_ERROR "AC-8 FAILED: Scene header not found: ${header_file}")
    endif()

    file(READ "${header_file}" content)
    get_filename_component(header_name "${header_file}" NAME)

    # Check 1: If file includes windows.h or has a platform guard, it MUST have an #else branch
    string(FIND "${content}" "windows.h" _pos_windows_h)
    if(NOT _pos_windows_h EQUAL -1)
        # Has windows.h — verify there's a #else branch
        string(FIND "${content}" "#else" _pos_else)
        if(_pos_else EQUAL -1)
            message(FATAL_ERROR "AC-8 FAILED: ${header_name} includes windows.h but has no #else branch — add #else / #include \"PlatformCompat.h\"")
        endif()

        # Verify the #else branch includes PlatformCompat.h
        string(REGEX MATCH "#else[^\n]*\n[^\n]*PlatformCompat" _match_else_compat "${content}")
        if(NOT _match_else_compat)
            message(FATAL_ERROR "AC-8 FAILED: ${header_name} has #else branch but it does not include PlatformCompat.h — fix the #else branch")
        endif()
    endif()

    # Check 2: No #ifdef _WIN32 wrapping class members or method declarations
    # (only include-selection is allowed — not member/method guards)
    # Heuristic: look for #ifdef _WIN32 followed by non-include content (class member patterns)
    string(REGEX MATCH "#ifdef _WIN32[^\n]*\n[ \t]*(virtual|inline|static|BOOL|DWORD|WORD|HANDLE)[^\n]*" _match_member_guard "${content}")
    if(_match_member_guard)
        message(FATAL_ERROR "AC-8 FAILED: ${header_name} has #ifdef _WIN32 wrapping a class member or method declaration")
    endif()

    message(STATUS "AC-8 PASSED: ${header_name} has correct include-selection pattern with #else / PlatformCompat.h")
endforeach()

message(STATUS "AC-8 PASSED: All 4 scene headers have correct windows.h include-selection with #else branch")
