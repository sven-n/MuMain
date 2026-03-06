# Story 2.1.1: AC-7 — SDL3 sources guarded by MU_ENABLE_SDL3 in CMakeLists.txt
# RED PHASE: SDL3 window/event loop sources not yet added to CMake.
#
# Validates that:
# 1. SDLWindow.cpp and SDLEventLoop.cpp are conditionally included via if(MU_ENABLE_SDL3)
# 2. Win32Window.cpp and Win32EventLoop.cpp exist for Windows path
# 3. MuPlatform.cpp is added unconditionally

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CMAKELISTS_FILE)
    message(FATAL_ERROR "CMAKELISTS_FILE must be set to the path of src/CMakeLists.txt")
endif()

file(READ "${CMAKELISTS_FILE}" cmake_content)

# --- Check 1: SDL3 window sources guarded by MU_ENABLE_SDL3 ---
string(FIND "${cmake_content}" "SDLWindow.cpp" sdl_window_pos)
if(sdl_window_pos EQUAL -1)
    message(FATAL_ERROR "AC-7 FAIL: SDLWindow.cpp not found in CMakeLists.txt — SDL3 window backend not added")
endif()

# Verify SDLWindow.cpp appears inside an if(MU_ENABLE_SDL3) block
# Look for the pattern: if(MU_ENABLE_SDL3) ... SDLWindow.cpp
string(REGEX MATCH "if[ \t]*\\([ \t]*MU_ENABLE_SDL3[ \t]*\\)[^)]*SDLWindow\\.cpp" sdl_guard_match "${cmake_content}")
if("${sdl_guard_match}" STREQUAL "")
    message(FATAL_ERROR "AC-7 FAIL: SDLWindow.cpp not guarded by if(MU_ENABLE_SDL3)")
endif()

# --- Check 2: SDL3 event loop sources guarded ---
string(FIND "${cmake_content}" "SDLEventLoop.cpp" sdl_event_pos)
if(sdl_event_pos EQUAL -1)
    message(FATAL_ERROR "AC-7 FAIL: SDLEventLoop.cpp not found in CMakeLists.txt — SDL3 event loop backend not added")
endif()

string(REGEX MATCH "if[ \t]*\\([ \t]*MU_ENABLE_SDL3[ \t]*\\)[^)]*SDLEventLoop\\.cpp" sdl_event_guard_match "${cmake_content}")
if("${sdl_event_guard_match}" STREQUAL "")
    message(FATAL_ERROR "AC-7 FAIL: SDLEventLoop.cpp not guarded by if(MU_ENABLE_SDL3)")
endif()

# --- Check 3: MuPlatform.cpp exists (unconditional) ---
string(FIND "${cmake_content}" "MuPlatform.cpp" platform_pos)
if(platform_pos EQUAL -1)
    message(FATAL_ERROR "AC-7 FAIL: MuPlatform.cpp not found in CMakeLists.txt — platform facade not added")
endif()

# --- Check 4: Win32 backend sources exist ---
string(FIND "${cmake_content}" "Win32Window.cpp" win32_window_pos)
if(win32_window_pos EQUAL -1)
    message(FATAL_ERROR "AC-7 FAIL: Win32Window.cpp not found in CMakeLists.txt — Win32 window backend not added")
endif()

string(FIND "${cmake_content}" "Win32EventLoop.cpp" win32_event_pos)
if(win32_event_pos EQUAL -1)
    message(FATAL_ERROR "AC-7 FAIL: Win32EventLoop.cpp not found in CMakeLists.txt — Win32 event loop backend not added")
endif()

message(STATUS "AC-7 PASS: SDL3 sources guarded by MU_ENABLE_SDL3, Win32 sources present, MuPlatform.cpp unconditional")
