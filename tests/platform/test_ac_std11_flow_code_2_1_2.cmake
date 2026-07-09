# Story 2.1.2: SDL3 Window Focus & Display Management
# AC-STD-11: Verify flow code VS1-SDL-WINDOW-FOCUS appears in SDLEventLoop.cpp
# GREEN PHASE: Focus/minimize/restore handlers include flow code in log output.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PLATFORM_DIR)
    message(FATAL_ERROR "PLATFORM_DIR must be set")
endif()

set(SDL_EVENT_LOOP_FILE "${PLATFORM_DIR}/sdl3/SDLEventLoop.cpp")

if(NOT EXISTS "${SDL_EVENT_LOOP_FILE}")
    message(FATAL_ERROR "SDLEventLoop.cpp not found at: ${SDL_EVENT_LOOP_FILE}")
endif()

file(READ "${SDL_EVENT_LOOP_FILE}" FILE_CONTENT)

string(FIND "${FILE_CONTENT}" "VS1-SDL-WINDOW-FOCUS" FOUND_POS)

if(FOUND_POS EQUAL -1)
    message(FATAL_ERROR
        "AC-STD-11 FAILED: Flow code 'VS1-SDL-WINDOW-FOCUS' not found in SDLEventLoop.cpp. "
        "The focus/minimize/restore handlers must include the flow code in log output "
        "via g_ErrorReport.Write() or g_ConsoleDebug->Write()."
    )
endif()

message(STATUS "AC-STD-11 PASSED: Flow code 'VS1-SDL-WINDOW-FOCUS' found in SDLEventLoop.cpp")
