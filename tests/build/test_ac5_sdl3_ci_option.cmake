# AC-5: Validate MU_ENABLE_SDL3 option exists for CI gating
#
# Story: 1.3.1 - SDL3 Dependency Integration
# RED phase: This test FAILS until MU_ENABLE_SDL3 option is added.
#
# Expected in MuMain/src/CMakeLists.txt:
#   - option(MU_ENABLE_SDL3 ...) or set(MU_ENABLE_SDL3 ...) definition
#   - FetchContent block is gated behind if(MU_ENABLE_SDL3)
#   - SDL3 link targets are gated behind if(MU_ENABLE_SDL3)
#
# This ensures MinGW CI can disable SDL3 with -DMU_ENABLE_SDL3=OFF if needed.

cmake_minimum_required(VERSION 3.25)

# CMAKELISTS_FILE is passed via -D from the CTest invocation
if(NOT DEFINED CMAKELISTS_FILE)
    message(FATAL_ERROR "AC-5: CMAKELISTS_FILE variable not defined")
endif()

if(NOT EXISTS "${CMAKELISTS_FILE}")
    message(FATAL_ERROR "AC-5: CMakeLists.txt does not exist: ${CMAKELISTS_FILE}")
endif()

file(READ "${CMAKELISTS_FILE}" CMAKE_CONTENT)

# --- Check 1: MU_ENABLE_SDL3 option exists ---
string(REGEX MATCH "option[\\(\\s]*MU_ENABLE_SDL3" _match_option "${CMAKE_CONTENT}")
if(NOT _match_option)
    message(FATAL_ERROR "AC-5: option(MU_ENABLE_SDL3 ...) not found in CMakeLists.txt")
endif()

# --- Check 2: FetchContent block is gated behind if(MU_ENABLE_SDL3) ---
# Look for the pattern: if(MU_ENABLE_SDL3) ... FetchContent ... endif()
string(FIND "${CMAKE_CONTENT}" "if(MU_ENABLE_SDL3)" _pos_if_enable)
if(_pos_if_enable EQUAL -1)
    # Also try with spaces
    string(FIND "${CMAKE_CONTENT}" "if (MU_ENABLE_SDL3)" _pos_if_enable)
endif()
if(_pos_if_enable EQUAL -1)
    message(FATAL_ERROR "AC-5: FetchContent block not gated behind if(MU_ENABLE_SDL3)")
endif()

# Verify FetchContent_Declare appears after the if(MU_ENABLE_SDL3)
string(FIND "${CMAKE_CONTENT}" "FetchContent_Declare" _pos_fc_declare)
if(_pos_fc_declare EQUAL -1)
    message(FATAL_ERROR "AC-5: FetchContent_Declare not found")
endif()

if(_pos_fc_declare LESS _pos_if_enable)
    message(FATAL_ERROR "AC-5: FetchContent_Declare appears BEFORE if(MU_ENABLE_SDL3) -- it must be inside the guard")
endif()

# --- Check 3: SDL3 target_link_libraries are gated ---
# The link commands for MUPlatform and MURenderFX with SDL3 should be
# inside an if(MU_ENABLE_SDL3) block
string(REGEX MATCHALL "if[\\(\\s]*MU_ENABLE_SDL3[\\)\\s]" _all_if_guards "${CMAKE_CONTENT}")
list(LENGTH _all_if_guards _guard_count)
if(_guard_count LESS 1)
    message(FATAL_ERROR "AC-5: Expected at least 1 if(MU_ENABLE_SDL3) guard, found ${_guard_count}")
endif()

message(STATUS "AC-5: PASSED -- MU_ENABLE_SDL3 option exists with ${_guard_count} guard block(s)")
