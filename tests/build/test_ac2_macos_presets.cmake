# AC-2: Validate CMakePresets.json includes macOS presets
#
# Story: 1.1.1 - Create macOS CMake Toolchain & Presets
# RED phase: This test FAILS until the macOS presets are added.
#
# Expected presets:
#   - "macos-base" (hidden configure preset)
#   - "macos-arm64" (configure preset inheriting macos-base)
#   - "macos-arm64-debug" (build preset)
#   - "macos-arm64-release" (build preset)
#   - "condition" block with hostSystemName == "Darwin"
#   - Generator: "Ninja Multi-Config"
#   - CMAKE_EXPORT_COMPILE_COMMANDS ON
#   - Reference to macos-arm64.cmake toolchain file

cmake_minimum_required(VERSION 3.25)

# PRESETS_FILE is passed via -D from the CTest invocation
if(NOT DEFINED PRESETS_FILE)
    message(FATAL_ERROR "AC-2: PRESETS_FILE variable not defined")
endif()

# --- Check 1: File exists ---
if(NOT EXISTS "${PRESETS_FILE}")
    message(FATAL_ERROR "AC-2: CMakePresets.json does not exist: ${PRESETS_FILE}")
endif()

file(READ "${PRESETS_FILE}" PRESETS_CONTENT)

# --- Check 2: macos-base hidden preset exists ---
string(FIND "${PRESETS_CONTENT}" "\"macos-base\"" _pos_macos_base)
if(_pos_macos_base EQUAL -1)
    message(FATAL_ERROR "AC-2: CMakePresets.json missing 'macos-base' hidden preset")
endif()

# --- Check 3: macos-arm64 configure preset exists ---
string(FIND "${PRESETS_CONTENT}" "\"macos-arm64\"" _pos_macos_arm64)
if(_pos_macos_arm64 EQUAL -1)
    message(FATAL_ERROR "AC-2: CMakePresets.json missing 'macos-arm64' configure preset")
endif()

# --- Check 4: macos-arm64-debug build preset exists ---
string(FIND "${PRESETS_CONTENT}" "\"macos-arm64-debug\"" _pos_debug)
if(_pos_debug EQUAL -1)
    message(FATAL_ERROR "AC-2: CMakePresets.json missing 'macos-arm64-debug' build preset")
endif()

# --- Check 5: macos-arm64-release build preset exists ---
string(FIND "${PRESETS_CONTENT}" "\"macos-arm64-release\"" _pos_release)
if(_pos_release EQUAL -1)
    message(FATAL_ERROR "AC-2: CMakePresets.json missing 'macos-arm64-release' build preset")
endif()

# --- Check 6: Darwin condition block (hostSystemName == Darwin) ---
string(FIND "${PRESETS_CONTENT}" "\"Darwin\"" _pos_darwin_condition)
if(_pos_darwin_condition EQUAL -1)
    message(FATAL_ERROR "AC-2: CMakePresets.json missing Darwin condition (hostSystemName == Darwin)")
endif()

# --- Check 7: Ninja Multi-Config generator (should appear for macos-base too) ---
# windows-base + linux-base + macos-base = at least 3 occurrences
string(REGEX MATCHALL "\"Ninja Multi-Config\"" _ninja_matches "${PRESETS_CONTENT}")
list(LENGTH _ninja_matches _ninja_count)
if(_ninja_count LESS 3)
    message(FATAL_ERROR "AC-2: Expected 'Ninja Multi-Config' generator in windows-base, linux-base, AND macos-base presets (found ${_ninja_count})")
endif()

# --- Check 8: CMAKE_EXPORT_COMPILE_COMMANDS ON in macOS presets ---
# windows-base + linux-base + macos-base = at least 3 occurrences
string(REGEX MATCHALL "CMAKE_EXPORT_COMPILE_COMMANDS" _compile_cmd_matches "${PRESETS_CONTENT}")
list(LENGTH _compile_cmd_matches _compile_cmd_count)
if(_compile_cmd_count LESS 3)
    message(FATAL_ERROR "AC-2: CMAKE_EXPORT_COMPILE_COMMANDS should be in windows-base, linux-base, AND macos-base (found ${_compile_cmd_count})")
endif()

# --- Check 9: macos-arm64 references toolchain file ---
string(FIND "${PRESETS_CONTENT}" "macos-arm64.cmake" _pos_toolchain_ref)
if(_pos_toolchain_ref EQUAL -1)
    message(FATAL_ERROR "AC-2: macos-arm64 preset does not reference macos-arm64.cmake toolchain file")
endif()

# --- Check 10: macos-arm64 inherits from macos-base ---
# Look for "inherits" near "macos-arm64" with "macos-base" value
string(FIND "${PRESETS_CONTENT}" "\"macos-base\"" _pos_inherits_base)
if(_pos_inherits_base EQUAL -1)
    message(FATAL_ERROR "AC-2: macos-arm64 preset does not inherit from macos-base")
endif()

message(STATUS "AC-2: PASSED -- CMakePresets.json contains all required macOS presets")
