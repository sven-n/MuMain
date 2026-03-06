# Story 2.2.1: SDL3 Keyboard Input Migration [VS1-SDL-INPUT-KEYBOARD]
# AC-STD-3: Regression test — no raw GetAsyncKeyState calls outside Platform/
# RED PHASE: Verifies that all GetAsyncKeyState calls in non-Platform/ game logic
#            files go through the shim transparently (zero call-site changes needed).
#
# The shim in PlatformCompat.h (inside #else // !_WIN32) handles all calls.
# On Windows, Win32 GetAsyncKeyState is used directly — no change.
# On non-Windows: the shim intercepts all calls. No new direct calls permitted.
#
# ALLOWED locations for GetAsyncKeyState:
#   - MuMain/src/source/Platform/PlatformCompat.h  (shim definition)
#   - MuMain/src/source/Platform/**                (platform abstraction layer)
#   - MuMain/src/source/ThirdParty/UIControls.cpp  (ThirdParty/ excluded from lint)
#
# FORBIDDEN: Any new GetAsyncKeyState call in game logic files OUTSIDE Platform/
# This test reports a WARNING (not FATAL_ERROR) for existing calls — they are
# expected to compile through the shim. It only FATAL_ERRORs if new calls are
# found in files not in the known-call-site list.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be set (path to MuMain/src/source)")
endif()

# Known call-site files (8 game logic files identified in story dev notes)
# These are ALLOWED to call GetAsyncKeyState — the shim handles them transparently.
set(KNOWN_CALL_SITES
    "NewUICommon.cpp"
    "ZzzInterface.cpp"
    "SceneCommon.cpp"
    "SceneManager.cpp"
    "UIGuildInfo.cpp"
    "CameraUtility.cpp"
    "Winmain.cpp"
    "UIControls.cpp"
)

# Directories that are ALLOWED to contain GetAsyncKeyState
set(ALLOWED_DIRS
    "Platform"
    "ThirdParty"
)

# Search all .cpp and .h files for GetAsyncKeyState (excluding Platform/ and ThirdParty/)
file(GLOB_RECURSE ALL_SOURCE_FILES
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
)

set(UNEXPECTED_FILES "")
set(KNOWN_FILES_FOUND "")

foreach(FILE_PATH ${ALL_SOURCE_FILES})
    # Read file content
    file(READ "${FILE_PATH}" CONTENT)
    string(FIND "${CONTENT}" "GetAsyncKeyState" FOUND_POS)

    if(NOT FOUND_POS EQUAL -1)
        # Check if this is in an allowed directory
        set(IS_ALLOWED FALSE)
        foreach(ALLOWED_DIR ${ALLOWED_DIRS})
            string(FIND "${FILE_PATH}" "/${ALLOWED_DIR}/" IN_ALLOWED_DIR)
            if(NOT IN_ALLOWED_DIR EQUAL -1)
                set(IS_ALLOWED TRUE)
                break()
            endif()
        endforeach()

        if(IS_ALLOWED)
            # Platform/ or ThirdParty/ — allowed
            continue()
        endif()

        # Check if this is a known call-site file
        get_filename_component(FILE_NAME "${FILE_PATH}" NAME)
        set(IS_KNOWN FALSE)
        foreach(KNOWN_FILE ${KNOWN_CALL_SITES})
            if("${FILE_NAME}" STREQUAL "${KNOWN_FILE}")
                set(IS_KNOWN TRUE)
                break()
            endif()
        endforeach()

        if(IS_KNOWN)
            list(APPEND KNOWN_FILES_FOUND "${FILE_PATH}")
        else()
            list(APPEND UNEXPECTED_FILES "${FILE_PATH}")
        endif()
    endif()
endforeach()

# Report known call sites (informational — these compile through the shim)
list(LENGTH KNOWN_FILES_FOUND KNOWN_COUNT)
if(KNOWN_COUNT GREATER 0)
    message(STATUS "AC-STD-3: Found ${KNOWN_COUNT} known call-site file(s) — handled by shim:")
    foreach(F ${KNOWN_FILES_FOUND})
        message(STATUS "  [SHIM OK] ${F}")
    endforeach()
endif()

# Fail if unexpected new call sites are found
list(LENGTH UNEXPECTED_FILES UNEXPECTED_COUNT)
if(UNEXPECTED_COUNT GREATER 0)
    message(FATAL_ERROR
        "AC-STD-3 FAILED: Found ${UNEXPECTED_COUNT} unexpected GetAsyncKeyState call(s) "
        "outside Platform/ and known game logic files. "
        "New direct GetAsyncKeyState calls are FORBIDDEN outside PlatformCompat.h. "
        "Use the shim or CInput::Instance().IsKeyDown() instead.\n"
        "Unexpected files:\n${UNEXPECTED_FILES}"
    )
endif()

message(STATUS
    "AC-STD-3 PASSED: No unexpected GetAsyncKeyState calls outside Platform/ and known call sites. "
    "[VS1-SDL-INPUT-KEYBOARD]"
)
