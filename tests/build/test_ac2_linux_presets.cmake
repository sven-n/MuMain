# AC-2: Validate CMakePresets.json includes Linux presets
#
# Story: 1.1.2 - Create Linux CMake Toolchain & Presets
# RED phase: This test FAILS until the Linux presets are added.
#
# Expected presets:
#   - "linux-base" (hidden configure preset)
#   - "linux-x64" (configure preset inheriting linux-base)
#   - "linux-x64-debug" (build preset)
#   - "linux-x64-release" (build preset)
#   - "condition" block with hostSystemName == "Linux"
#   - Generator: "Ninja Multi-Config"
#   - CMAKE_EXPORT_COMPILE_COMMANDS ON

cmake_minimum_required(VERSION 3.25)

# PRESETS_FILE is passed via -D from the CTest invocation
if(NOT DEFINED PRESETS_FILE)
    message(FATAL_ERROR "AC-2: PRESETS_FILE variable not defined")
endif()

# --- Check 1: File exists and is valid JSON ---
if(NOT EXISTS "${PRESETS_FILE}")
    message(FATAL_ERROR "AC-2: CMakePresets.json does not exist: ${PRESETS_FILE}")
endif()

file(READ "${PRESETS_FILE}" PRESETS_CONTENT)

# --- Check 2: linux-base hidden preset exists ---
string(FIND "${PRESETS_CONTENT}" "\"linux-base\"" _pos_linux_base)
if(_pos_linux_base EQUAL -1)
    message(FATAL_ERROR "AC-2: CMakePresets.json missing 'linux-base' hidden preset")
endif()

# --- Check 3: linux-x64 configure preset exists ---
string(FIND "${PRESETS_CONTENT}" "\"linux-x64\"" _pos_linux_x64)
if(_pos_linux_x64 EQUAL -1)
    message(FATAL_ERROR "AC-2: CMakePresets.json missing 'linux-x64' configure preset")
endif()

# --- Check 4: linux-x64-debug build preset exists ---
string(FIND "${PRESETS_CONTENT}" "\"linux-x64-debug\"" _pos_debug)
if(_pos_debug EQUAL -1)
    message(FATAL_ERROR "AC-2: CMakePresets.json missing 'linux-x64-debug' build preset")
endif()

# --- Check 5: linux-x64-release build preset exists ---
string(FIND "${PRESETS_CONTENT}" "\"linux-x64-release\"" _pos_release)
if(_pos_release EQUAL -1)
    message(FATAL_ERROR "AC-2: CMakePresets.json missing 'linux-x64-release' build preset")
endif()

# --- Check 6: Linux condition block (hostSystemName == Linux) ---
string(FIND "${PRESETS_CONTENT}" "\"Linux\"" _pos_linux_condition)
if(_pos_linux_condition EQUAL -1)
    message(FATAL_ERROR "AC-2: CMakePresets.json missing Linux condition (hostSystemName == Linux)")
endif()

# --- Check 7: Ninja Multi-Config generator ---
# The linux-base preset should use "Ninja Multi-Config"
# We already know the windows-base uses it; verify it appears more than once (for linux-base too)
string(REGEX MATCHALL "\"Ninja Multi-Config\"" _ninja_matches "${PRESETS_CONTENT}")
list(LENGTH _ninja_matches _ninja_count)
if(_ninja_count LESS 2)
    message(FATAL_ERROR "AC-2: Expected 'Ninja Multi-Config' generator in both windows-base and linux-base presets (found ${_ninja_count})")
endif()

# --- Check 8: CMAKE_EXPORT_COMPILE_COMMANDS ON in Linux presets ---
# This should appear at least twice (windows-base + linux-base)
string(REGEX MATCHALL "CMAKE_EXPORT_COMPILE_COMMANDS" _compile_cmd_matches "${PRESETS_CONTENT}")
list(LENGTH _compile_cmd_matches _compile_cmd_count)
if(_compile_cmd_count LESS 2)
    message(FATAL_ERROR "AC-2: CMAKE_EXPORT_COMPILE_COMMANDS should be in both windows-base and linux-base (found ${_compile_cmd_count})")
endif()

# --- Check 9: linux-x64 references toolchain file ---
string(FIND "${PRESETS_CONTENT}" "linux-x64.cmake" _pos_toolchain_ref)
if(_pos_toolchain_ref EQUAL -1)
    message(FATAL_ERROR "AC-2: linux-x64 preset does not reference linux-x64.cmake toolchain file")
endif()

# --- Check 10: Existing Windows presets still present (regression) ---
string(FIND "${PRESETS_CONTENT}" "\"windows-base\"" _pos_win_base)
if(_pos_win_base EQUAL -1)
    message(FATAL_ERROR "AC-2: REGRESSION -- windows-base preset is missing from CMakePresets.json")
endif()

string(FIND "${PRESETS_CONTENT}" "\"windows-x64\"" _pos_win_x64)
if(_pos_win_x64 EQUAL -1)
    message(FATAL_ERROR "AC-2: REGRESSION -- windows-x64 preset is missing from CMakePresets.json")
endif()

message(STATUS "AC-2: PASSED -- CMakePresets.json contains all required Linux presets")
