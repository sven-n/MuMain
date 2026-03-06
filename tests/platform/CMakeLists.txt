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
