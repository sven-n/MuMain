# Story 1.2.1: Platform Abstraction Headers — Test Registration
# GREEN PHASE: All platform headers implemented and tests passing.

set(PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../src/source/Platform")

# ---- Catch2 unit tests (linked into MuTests in parent) ----
# NOTE: test_platform_types.cpp, test_platform_compat.cpp, and
# test_platform_keys.cpp are added to the MuTests target in the
# parent CMakeLists.txt. They are not registered separately here.

# ---- CMake script-mode validation tests ----

# AC-3: Verify all platform headers use #pragma once
add_test(
    NAME "AC-3:platform-pragma-once"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_DIR=${PLATFORM_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_pragma_once.cmake
)

# AC-4: Verify all platform headers exist (file presence check)
add_test(
    NAME "AC-4:platform-header-existence"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_DIR=${PLATFORM_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac4_header_compilation.cmake
)

# AC-5 + AC-STD-3: Verify no new #ifdef _WIN32 leaked into game logic
add_test(
    NAME "AC-5:no-game-logic-ifdef"
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test_ac5_no_game_logic_changes.sh
        "${CMAKE_CURRENT_SOURCE_DIR}/../../src/source"
)

# ---- Story 1.2.2: Platform Library Backends — RED PHASE ----

# AC-STD-3,AC-1,AC-6: Verify PlatformLibrary.h is platform-neutral
add_test(
    NAME "AC-STD-3:platform-library-header-neutral"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_DIR=${PLATFORM_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_1_2_2_header_neutral.cmake
)

# AC-4,AC-2,AC-3: Verify CMake backend selection and backend files exist
add_test(
    NAME "AC-4:platform-library-cmake-backend"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${CMAKE_CURRENT_SOURCE_DIR}/../../src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_1_2_2_cmake_backend.cmake
)

# ---- Story 2.1.1: SDL3 Window Creation & Event Loop — RED PHASE ----

# AC-1,AC-2,AC-5: Platform interface headers exist with correct declarations
add_test(
    NAME "2.1.1-AC-1:platform-interfaces-exist"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_DIR=${PLATFORM_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_platform_interfaces.cmake
)

# AC-7: SDL3 sources guarded by MU_ENABLE_SDL3 in CMakeLists.txt
add_test(
    NAME "2.1.1-AC-7:cmake-sdl3-guard"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${CMAKE_CURRENT_SOURCE_DIR}/../../src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac7_cmake_sdl3_guard.cmake
)

# AC-7: All SDL3 source files contain MU_ENABLE_SDL3 ifdef guard
add_test(
    NAME "2.1.1-AC-7:sdl3-ifdef-guard"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_DIR=${PLATFORM_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac7_sdl3_ifdef_guard.cmake
)

# AC-STD-11: Flow code and story references in test files
add_test(
    NAME "2.1.1-AC-STD-11:flow-code"
    COMMAND ${CMAKE_COMMAND}
        -DTEST_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code.cmake
)

# ---- Story 2.1.2: SDL3 Window Focus & Display Management — RED PHASE ----

# AC-STD-11: Flow code VS1-SDL-WINDOW-FOCUS in SDLEventLoop.cpp
add_test(
    NAME "2.1.2-AC-STD-11:flow-code-focus"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_DIR=${PLATFORM_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_2_1_2.cmake
)

# ---- Story 2.2.1: SDL3 Keyboard Input Migration — RED PHASE ----
# [VS1-SDL-INPUT-KEYBOARD]
# Catch2 unit tests are registered via MuTests in the parent CMakeLists.txt.
# (test_platform_input.cpp — VK->scancode mapping, GetAsyncKeyState shim, HIBYTE)

# AC-STD-11: Flow code VS1-SDL-INPUT-KEYBOARD in PlatformCompat.h
add_test(
    NAME "2.2.1-AC-STD-11:flow-code-keyboard"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_DIR=${PLATFORM_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_2_2_1.cmake
)

# AC-STD-3: No raw GetAsyncKeyState calls outside Platform/ and known call sites
add_test(
    NAME "2.2.1-AC-STD-3:no-raw-getasynckeystate"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/../../src/source
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std3_no_raw_getasynckeystate.cmake
)

# ---- Story 2.2.2: SDL3 Mouse Input Migration — RED PHASE ----
# [VS1-SDL-INPUT-MOUSE]
# Catch2 unit tests are registered via MuTests in the parent CMakeLists.txt.
# (test_platform_mouse.cpp — button state transitions, wheel sign, coord normalization,
#  GetDoubleClickTime shim, VK_LBUTTON/RBUTTON/MBUTTON GetAsyncKeyState shim, POINT struct)

# AC-STD-11: Flow code VS1-SDL-INPUT-MOUSE in SDLEventLoop.cpp
add_test(
    NAME "2.2.2-AC-STD-11:flow-code-mouse"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_DIR=${PLATFORM_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_2_2_2.cmake
)

# AC-STD-3: No raw Win32 mouse WM_* patterns outside Winmain.cpp and Platform/
add_test(
    NAME "2.2.2-AC-STD-3:no-raw-win32-mouse"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/../../src/source
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std3_no_raw_win32_mouse.cmake
)

# ---- Story 2.2.3: SDL3 Text Input Migration — RED PHASE ----
# [VS1-SDL-INPUT-TEXT]
# Catch2 unit tests are registered via MuTests in the parent CMakeLists.txt.
# (test_platform_text_input.cpp — UTF-8 decoding, text buffer append/truncation,
#  backspace edge detection, NUMBERONLY filter, MuSdlUtf8NextChar correctness)

# AC-STD-11: Flow code VS1-SDL-INPUT-TEXT in SDLEventLoop.cpp
add_test(
    NAME "2.2.3-AC-STD-11:flow-code-text-input"
    COMMAND ${CMAKE_COMMAND}
        -DPLATFORM_DIR=${PLATFORM_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_2_2_3.cmake
)

# AC-STD-3: No raw Win32 IME API calls outside Platform/ and ThirdParty/
add_test(
    NAME "2.2.3-AC-STD-3:no-raw-imm-apis"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/../../src/source
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std3_no_raw_imm.cmake
)

# ---- Story 7.1.2: POSIX Signal Handlers for Crash Diagnostics — RED PHASE ----
# [VS0-QUAL-SIGNAL-HANDLERS]
# AC-STD-3: sigaction/signal() calls must appear ONLY in Platform/posix/ files.
# Catch2 unit tests (install verification) registered via MuTests in parent CMakeLists.txt.
# (test_posix_signal_handlers.cpp — SA_SIGACTION flag check for SIGSEGV/SIGABRT/SIGBUS)
