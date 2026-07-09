# Story 2.2.1: SDL3 Keyboard Input Migration [VS1-SDL-INPUT-KEYBOARD]
# AC-STD-11: Verify flow code VS1-SDL-INPUT-KEYBOARD appears in PlatformCompat.h
# RED PHASE: Will FAIL until GetAsyncKeyState shim is implemented with flow code.
#
# Per story requirement: flow code VS1-SDL-INPUT-KEYBOARD must appear in:
#   - PlatformCompat.h (in the g_ErrorReport.Write unmapped-VK log message)
#   - Test names (satisfied by test_platform_input.cpp TEST_CASE strings)
#   - Story artifacts (satisfied by story.md and atdd.md)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PLATFORM_DIR)
    message(FATAL_ERROR "PLATFORM_DIR must be set (path to MuMain/src/source/Platform)")
endif()

set(PLATFORM_COMPAT_FILE "${PLATFORM_DIR}/PlatformCompat.h")

if(NOT EXISTS "${PLATFORM_COMPAT_FILE}")
    message(FATAL_ERROR
        "AC-STD-11 FAILED: PlatformCompat.h not found at: ${PLATFORM_COMPAT_FILE}"
    )
endif()

file(READ "${PLATFORM_COMPAT_FILE}" FILE_CONTENT)

string(FIND "${FILE_CONTENT}" "VS1-SDL-INPUT-KEYBOARD" FOUND_POS)

if(FOUND_POS EQUAL -1)
    message(FATAL_ERROR
        "AC-STD-11 FAILED: Flow code 'VS1-SDL-INPUT-KEYBOARD' not found in PlatformCompat.h. "
        "The GetAsyncKeyState shim must include the flow code in the unmapped-VK error log: "
        "g_ErrorReport.Write(L\"MU_ERR_INPUT_UNMAPPED_VK [VS1-SDL-INPUT-KEYBOARD]: ...\"). "
        "File checked: ${PLATFORM_COMPAT_FILE}"
    )
endif()

message(STATUS "AC-STD-11 PASSED: Flow code 'VS1-SDL-INPUT-KEYBOARD' found in PlatformCompat.h")
