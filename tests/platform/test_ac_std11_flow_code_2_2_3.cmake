# Story 2.2.3: SDL3 Text Input Migration [VS1-SDL-INPUT-TEXT]
# AC-STD-11: Verify flow code VS1-SDL-INPUT-TEXT appears in SDLEventLoop.cpp
# RED PHASE: Will FAIL until SDL text input handler is implemented with flow code.
#
# Per story requirement, flow code VS1-SDL-INPUT-TEXT must appear in:
#   - SDLEventLoop.cpp (in SDL_EVENT_TEXT_INPUT handler and/or error log messages)
#   - Test names (satisfied by test_platform_text_input.cpp TEST_CASE strings)
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

string(FIND "${FILE_CONTENT}" "VS1-SDL-INPUT-TEXT" FOUND_POS)

if(FOUND_POS EQUAL -1)
    message(FATAL_ERROR
        "AC-STD-11 FAILED: Flow code 'VS1-SDL-INPUT-TEXT' not found in SDLEventLoop.cpp. "
        "The SDL_EVENT_TEXT_INPUT handler must include the flow code as a comment or in "
        "error log messages: g_ErrorReport.Write(L\"... [VS1-SDL-INPUT-TEXT] ...\"). "
        "File checked: ${SDL_EVENT_LOOP_FILE}"
    )
endif()

message(STATUS "AC-STD-11 PASSED: Flow code 'VS1-SDL-INPUT-TEXT' found in SDLEventLoop.cpp")
