# AC-4: Validate Windows MSVC presets are unchanged after macOS preset addition
#
# Story: 1.1.1 - Create macOS CMake Toolchain & Presets
# RED phase: This test validates regression safety -- it passes if Windows
# presets exist and are structurally intact after macOS presets are added.
#
# Expected: All Windows presets remain present and valid:
#   - windows-base (hidden)
#   - windows-x86
#   - windows-x86-mueditor
#   - windows-x64
#   - windows-x64-mueditor
#   - windows-x86-debug, windows-x86-release (build presets)
#   - windows-x64-debug, windows-x64-release (build presets)
#   - windows-x86-mueditor-debug, windows-x86-mueditor-release (build presets)
#   - windows-x64-mueditor-debug, windows-x64-mueditor-release (build presets)
#   - Linux presets also remain intact (linux-base, linux-x64, etc.)

cmake_minimum_required(VERSION 3.25)

# PRESETS_FILE is passed via -D from the CTest invocation
if(NOT DEFINED PRESETS_FILE)
    message(FATAL_ERROR "AC-4: PRESETS_FILE variable not defined")
endif()

# --- Check 1: File exists ---
if(NOT EXISTS "${PRESETS_FILE}")
    message(FATAL_ERROR "AC-4: CMakePresets.json does not exist: ${PRESETS_FILE}")
endif()

file(READ "${PRESETS_FILE}" PRESETS_CONTENT)

# --- Check 2: Windows base preset ---
string(FIND "${PRESETS_CONTENT}" "\"windows-base\"" _pos)
if(_pos EQUAL -1)
    message(FATAL_ERROR "AC-4: REGRESSION -- windows-base preset is missing")
endif()

# --- Check 3: Windows configure presets ---
foreach(_preset "windows-x86" "windows-x86-mueditor" "windows-x64" "windows-x64-mueditor")
    string(FIND "${PRESETS_CONTENT}" "\"${_preset}\"" _pos)
    if(_pos EQUAL -1)
        message(FATAL_ERROR "AC-4: REGRESSION -- ${_preset} configure preset is missing")
    endif()
endforeach()

# --- Check 4: Windows build presets ---
foreach(_preset "windows-x86-debug" "windows-x86-release"
                "windows-x64-debug" "windows-x64-release"
                "windows-x86-mueditor-debug" "windows-x86-mueditor-release"
                "windows-x64-mueditor-debug" "windows-x64-mueditor-release")
    string(FIND "${PRESETS_CONTENT}" "\"${_preset}\"" _pos)
    if(_pos EQUAL -1)
        message(FATAL_ERROR "AC-4: REGRESSION -- ${_preset} build preset is missing")
    endif()
endforeach()

# --- Check 5: Windows condition still present ---
string(FIND "${PRESETS_CONTENT}" "\"Windows\"" _pos_win_condition)
if(_pos_win_condition EQUAL -1)
    message(FATAL_ERROR "AC-4: REGRESSION -- Windows condition (hostSystemName == Windows) is missing")
endif()

# --- Check 6: Linux presets also remain intact ---
string(FIND "${PRESETS_CONTENT}" "\"linux-base\"" _pos_linux_base)
if(_pos_linux_base EQUAL -1)
    message(FATAL_ERROR "AC-4: REGRESSION -- linux-base preset is missing")
endif()

string(FIND "${PRESETS_CONTENT}" "\"linux-x64\"" _pos_linux_x64)
if(_pos_linux_x64 EQUAL -1)
    message(FATAL_ERROR "AC-4: REGRESSION -- linux-x64 preset is missing")
endif()

string(FIND "${PRESETS_CONTENT}" "\"linux-x64-debug\"" _pos_linux_debug)
if(_pos_linux_debug EQUAL -1)
    message(FATAL_ERROR "AC-4: REGRESSION -- linux-x64-debug build preset is missing")
endif()

string(FIND "${PRESETS_CONTENT}" "\"linux-x64-release\"" _pos_linux_release)
if(_pos_linux_release EQUAL -1)
    message(FATAL_ERROR "AC-4: REGRESSION -- linux-x64-release build preset is missing")
endif()

# --- Check 7: macOS presets exist (confirms they were added without breaking Windows) ---
string(FIND "${PRESETS_CONTENT}" "\"macos-base\"" _pos_macos_base)
if(_pos_macos_base EQUAL -1)
    message(FATAL_ERROR "AC-4: macos-base preset is missing (macOS presets not added yet)")
endif()

string(FIND "${PRESETS_CONTENT}" "\"macos-arm64\"" _pos_macos_arm64)
if(_pos_macos_arm64 EQUAL -1)
    message(FATAL_ERROR "AC-4: macos-arm64 preset is missing (macOS presets not added yet)")
endif()

message(STATUS "AC-4: PASSED -- All Windows and Linux presets remain intact after macOS addition")
