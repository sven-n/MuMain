# Story 2.2.1: SDL3 Keyboard Input Migration [VS1-SDL-INPUT-KEYBOARD]
# AC-STD-11: Verify portable keyboard input remains centralized in KeyState.
#
# Per story requirement: flow code VS1-SDL-INPUT-KEYBOARD must appear in:
#   - PlatformCompat.h (in the g_ErrorReport.Write unmapped-VK log message)
#   - Test names (satisfied by test_platform_input.cpp TEST_CASE strings)
#   - Story artifacts (satisfied by story.md and atdd.md)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PLATFORM_DIR)
    message(FATAL_ERROR "PLATFORM_DIR must be set (path to MuMain/src/source/Platform)")
endif()

get_filename_component(SOURCE_DIR "${PLATFORM_DIR}" DIRECTORY)
set(KEY_STATE_FILE "${SOURCE_DIR}/Input/KeyState.cpp")

if(NOT EXISTS "${KEY_STATE_FILE}")
    message(FATAL_ERROR
        "AC-STD-11 FAILED: KeyState.cpp not found at: ${KEY_STATE_FILE}"
    )
endif()

file(READ "${KEY_STATE_FILE}" FILE_CONTENT)

string(FIND "${FILE_CONTENT}" "bool IsKeyDown(int virtualKey)" FOUND_POS)

if(FOUND_POS EQUAL -1)
    message(FATAL_ERROR
        "AC-STD-11 FAILED: portable IsKeyDown implementation missing from ${KEY_STATE_FILE}"
    )
endif()

message(STATUS "AC-STD-11 PASSED: portable keyboard input is centralized in KeyState.cpp")
