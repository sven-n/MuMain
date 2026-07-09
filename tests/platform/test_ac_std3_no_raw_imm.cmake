# Story 2.2.3: SDL3 Text Input Migration [VS1-SDL-INPUT-TEXT]
# AC-STD-3: Regression test — no raw Win32 IME API calls outside Platform/ or #ifdef _WIN32 guards.
#
# Verifies that ImmGetContext, ImmSetConversionStatus, ImmReleaseContext, ImmGetConversionStatus
# do NOT appear in game logic source files outside of:
#   - MuMain/src/source/Platform/       (platform abstraction layer — stubs allowed)
#   - MuMain/src/source/ThirdParty/     (legacy UIControls.cpp exception — guarded by _WIN32)
#
# Architecture: On non-Windows (SDL3) path, all IME APIs are stubbed to no-ops in
#               PlatformCompat.h (inside #else // !_WIN32 block). No game logic file
#               may call raw ImmGetContext etc. directly.
#
# ALLOWED locations:
#   - MuMain/src/source/Platform/**     (PlatformCompat.h contains the stubs)
#   - MuMain/src/source/ThirdParty/**   (UIControls.cpp uses IME APIs behind #ifdef _WIN32)
#
# FORBIDDEN: Any ImmGetContext / ImmSetConversionStatus in game logic files
#            outside Platform/ and ThirdParty/.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be set (path to MuMain/src/source)")
endif()

# Win32 IME patterns that must NOT appear unguarded in new game logic files
set(FORBIDDEN_PATTERNS
    "ImmGetContext"
    "ImmSetConversionStatus"
    "ImmReleaseContext"
    "ImmGetConversionStatus"
    "ImmSetCompositionWindow"
)

# Directories ALLOWED to contain IME API references (stubs and legacy UIControls)
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

foreach(FILE_PATH ${ALL_SOURCE_FILES})
    # Read file content
    file(READ "${FILE_PATH}" CONTENT)

    # Check if any forbidden IME pattern is present
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
        message(STATUS
            "AC-STD-3 [VS1-SDL-INPUT-TEXT]: IME pattern in allowed location (Platform/ or ThirdParty/): ${FILE_PATH}"
        )
        continue()
    endif()

    # Unexpected IME usage in game logic file
    list(APPEND UNEXPECTED_FILES "${FILE_PATH}")
endforeach()

# Fail if unexpected call sites are found
list(LENGTH UNEXPECTED_FILES UNEXPECTED_COUNT)
if(UNEXPECTED_COUNT GREATER 0)
    message(FATAL_ERROR
        "AC-STD-3 [VS1-SDL-INPUT-TEXT] FAILED: Found ${UNEXPECTED_COUNT} file(s) with raw Win32 "
        "IME API calls (ImmGetContext, ImmSetConversionStatus, etc.) outside Platform/ and ThirdParty/. "
        "All IME APIs on non-Windows paths must be stubbed in PlatformCompat.h (#else // !_WIN32 block). "
        "Game logic files must not call Imm* functions directly.\n"
        "Unexpected files:\n${UNEXPECTED_FILES}"
    )
endif()

message(STATUS
    "AC-STD-3 PASSED: No raw Win32 IME API calls outside Platform/ and ThirdParty/. "
    "[VS1-SDL-INPUT-TEXT]"
)
