# Story 2.2.2: SDL3 Mouse Input Migration [VS1-SDL-INPUT-MOUSE]
# AC-STD-11: Verify flow code VS1-SDL-INPUT-MOUSE appears in SDLEventLoop.cpp
# RED PHASE: Will FAIL until SDL mouse event handlers are implemented with flow code.
#
# Per story requirement: flow code VS1-SDL-INPUT-MOUSE must appear in:
#   - SDLEventLoop.cpp (in error log messages, e.g., MU_ERR_MOUSE_WARP_FAILED)
#   - Test names (satisfied by test_platform_mouse.cpp TEST_CASE strings)
#   - Story artifacts (satisfied by story.md and atdd.md)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PLATFORM_DIR)
    message(FATAL_ERROR "PLATFORM_DIR must be set (path to MuMain/src/source/Platform)")
endif()

set(SDL_EVENT_LOOP_FILE "${PLATFORM_DIR}/sdl3/SDLEventLoop.cpp")

if(NOT EXISTS "${SDL_EVENT_LOOP_FILE}")
    message(FATAL_ERROR
        "AC-STD-11 FAILED: SDLEventLoop.cpp not found at: ${SDL_EVENT_LOOP_FILE}"
    )
endif()

file(READ "${SDL_EVENT_LOOP_FILE}" FILE_CONTENT)

string(FIND "${FILE_CONTENT}" "VS1-SDL-INPUT-MOUSE" FOUND_POS)

if(FOUND_POS EQUAL -1)
    message(FATAL_ERROR
        "AC-STD-11 FAILED: Flow code 'VS1-SDL-INPUT-MOUSE' not found in SDLEventLoop.cpp. "
        "The mouse event handlers must include the flow code in error log messages: "
        "g_ErrorReport.Write(L\"MU_ERR_MOUSE_WARP_FAILED [VS1-SDL-INPUT-MOUSE]: ...\"). "
        "File checked: ${SDL_EVENT_LOOP_FILE}"
    )
endif()

message(STATUS "AC-STD-11 PASSED: Flow code 'VS1-SDL-INPUT-MOUSE' found in SDLEventLoop.cpp")
