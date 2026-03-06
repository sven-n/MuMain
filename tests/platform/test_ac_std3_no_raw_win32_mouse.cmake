# Story 2.2.2: SDL3 Mouse Input Migration [VS1-SDL-INPUT-MOUSE]
# AC-STD-3: Regression test — no raw Win32 mouse WM_* event handlers in new files.
#
# Verifies that:
#  1. WM_MOUSEMOVE, WM_LBUTTONDOWN, WM_RBUTTONDOWN, WM_MBUTTONDOWN,
#     WM_LBUTTONUP, WM_RBUTTONUP, WM_MBUTTONUP, WM_MOUSEWHEEL,
#     WM_LBUTTONDBLCLK, SetCapture, ReleaseCapture
#     only appear in ALLOWED locations (Winmain.cpp WndProc, Platform/ layer).
#  2. No new files outside the known-call-site list introduce these patterns.
#
# Architecture: On SDL3 path, all mouse state is fed from SDLEventLoop::PollEvents()
#               via SDL events. Win32 WM_* handlers remain ONLY in Winmain.cpp WndProc
#               for the Win32 path (zero regression).
#
# ALLOWED locations:
#   - MuMain/src/source/Main/Winmain.cpp          (legacy WndProc — Win32 path)
#   - MuMain/src/source/Platform/**               (platform abstraction layer)
#
# FORBIDDEN: Any new WM_MOUSEMOVE / SetCapture / ReleaseCapture in game logic files
#            outside Winmain.cpp and Platform/.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be set (path to MuMain/src/source)")
endif()

# Win32 mouse patterns that must NOT appear in new game logic files
set(FORBIDDEN_PATTERNS
    "WM_MOUSEMOVE"
    "WM_LBUTTONDOWN"
    "WM_RBUTTONDOWN"
    "WM_MBUTTONDOWN"
    "WM_LBUTTONUP"
    "WM_RBUTTONUP"
    "WM_MBUTTONUP"
    "WM_MOUSEWHEEL"
    "WM_LBUTTONDBLCLK"
    "SetCapture"
    "ReleaseCapture"
)

# Known files ALLOWED to contain Win32 mouse WM_* patterns
# (Winmain.cpp WndProc — Win32 path, retained for backward compatibility)
set(KNOWN_CALL_SITES
    "Winmain.cpp"
)

# Directories ALLOWED to contain any platform patterns
set(ALLOWED_DIRS
    "Platform"
    "ThirdParty"
)

# Collect all C++ source and header files
file(GLOB_RECURSE ALL_SOURCE_FILES
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
)

set(UNEXPECTED_FILES "")
set(KNOWN_FILES_FOUND "")

foreach(FILE_PATH ${ALL_SOURCE_FILES})
    # Read file content
    file(READ "${FILE_PATH}" CONTENT)

    # Check if any forbidden pattern is present
    set(HAS_FORBIDDEN FALSE)
    foreach(PATTERN ${FORBIDDEN_PATTERNS})
        string(FIND "${CONTENT}" "${PATTERN}" FOUND_POS)
        if(NOT FOUND_POS EQUAL -1)
            set(HAS_FORBIDDEN TRUE)
            break()
        endif()
    endforeach()

    if(NOT HAS_FORBIDDEN)
        continue()
    endif()

    # Check if file is in an allowed directory
    set(IS_ALLOWED FALSE)
    foreach(ALLOWED_DIR ${ALLOWED_DIRS})
        string(FIND "${FILE_PATH}" "/${ALLOWED_DIR}/" IN_ALLOWED_DIR)
        if(NOT IN_ALLOWED_DIR EQUAL -1)
            set(IS_ALLOWED TRUE)
            break()
        endif()
    endforeach()

    if(IS_ALLOWED)
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
endforeach()

# Report known call sites (informational — these are Win32 path handlers, not new code)
list(LENGTH KNOWN_FILES_FOUND KNOWN_COUNT)
if(KNOWN_COUNT GREATER 0)
    message(STATUS
        "AC-STD-3 [VS1-SDL-INPUT-MOUSE]: Found ${KNOWN_COUNT} known Win32 WndProc file(s) — expected:"
    )
    foreach(F ${KNOWN_FILES_FOUND})
        message(STATUS "  [WIN32-PATH OK] ${F}")
    endforeach()
endif()

# Fail if unexpected new call sites are found
list(LENGTH UNEXPECTED_FILES UNEXPECTED_COUNT)
if(UNEXPECTED_COUNT GREATER 0)
    message(FATAL_ERROR
        "AC-STD-3 [VS1-SDL-INPUT-MOUSE] FAILED: Found ${UNEXPECTED_COUNT} unexpected Win32 mouse "
        "WM_* pattern(s) outside Winmain.cpp and Platform/. "
        "All mouse state on SDL3 path must be fed from SDLEventLoop::PollEvents() via SDL events. "
        "No new WM_MOUSEMOVE / SetCapture / ReleaseCapture permitted in game logic files.\n"
        "Unexpected files:\n${UNEXPECTED_FILES}"
    )
endif()

message(STATUS
    "AC-STD-3 PASSED: No unexpected Win32 mouse WM_* patterns outside Winmain.cpp and Platform/. "
    "[VS1-SDL-INPUT-MOUSE]"
)
