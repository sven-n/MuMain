# Story 1.2.1: Platform Abstraction Headers — Test Registration
# Tests are in RED phase until PlatformTypes.h, PlatformKeys.h,
# and PlatformCompat.h are implemented.

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
