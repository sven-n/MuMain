# Flow Code: VS0-QUAL-BUILD-DOTNET
# Story 7.8.4: .NET Client Library Native Build
# AC-3: CMakePresets.json macos-base and linux-base no longer disable MU_ENABLE_DOTNET
#
# RED PHASE: Test FAILS until "MU_ENABLE_DOTNET": "OFF" is removed from
#            macos-base and linux-base preset cacheVariables.
#
# NOTE — Known conflict with story 7.4.1 test_ac8_dotnet_disabled_native_runners.cmake:
#   That test validates MU_ENABLE_DOTNET=OFF in native presets for CI runners.
#   When story 7.8.4 is implemented, the 7.4.1 test will fail and must be updated
#   or removed as part of this story's implementation.
#
# Expected: MuMain/CMakePresets.json:
#   - macos-base cacheVariables does NOT contain "MU_ENABLE_DOTNET": "OFF"
#   - linux-base cacheVariables does NOT contain "MU_ENABLE_DOTNET": "OFF"
#   - Windows presets are unchanged

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PRESETS_FILE)
    set(PRESETS_FILE "${CMAKE_CURRENT_LIST_DIR}/../../CMakePresets.json")
endif()

if(NOT EXISTS "${PRESETS_FILE}")
    message(FATAL_ERROR "AC-3 [7.8.4]: CMakePresets.json does not exist: ${PRESETS_FILE}")
endif()

file(READ "${PRESETS_FILE}" PRESETS_CONTENT)

# --- Check 1: macos-base must not have MU_ENABLE_DOTNET: OFF ---
# We check for the pattern near "macos-base" context
# Simple approach: the string "MU_ENABLE_DOTNET": "OFF" should not exist at all
# (since Windows never sets this — only macos-base and linux-base had it)
string(FIND "${PRESETS_CONTENT}" "\"MU_ENABLE_DOTNET\": \"OFF\"" _pos_dotnet_off)
if(NOT _pos_dotnet_off EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL [7.8.4]: \"MU_ENABLE_DOTNET\": \"OFF\" still present in CMakePresets.json — must be removed from macos-base and linux-base to enable .NET building on all platforms")
endif()

# --- Check 2: macos-base preset still exists (not accidentally deleted) ---
string(FIND "${PRESETS_CONTENT}" "macos-base" _pos_macos_base)
if(_pos_macos_base EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL [7.8.4]: 'macos-base' preset not found in CMakePresets.json — preset must exist but without MU_ENABLE_DOTNET: OFF")
endif()

# --- Check 3: linux-base preset still exists ---
string(FIND "${PRESETS_CONTENT}" "linux-base" _pos_linux_base)
if(_pos_linux_base EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL [7.8.4]: 'linux-base' preset not found in CMakePresets.json — preset must exist but without MU_ENABLE_DOTNET: OFF")
endif()

# --- Check 4: windows presets are untouched (MU_ENABLE_DOTNET not disabled there) ---
# Windows never had MU_ENABLE_DOTNET: OFF, so this is implicitly satisfied above.
# Verify windows-base or windows-x64 preset still present.
string(FIND "${PRESETS_CONTENT}" "windows" _pos_windows)
if(_pos_windows EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL [7.8.4]: No windows preset found in CMakePresets.json — Windows presets must remain unchanged")
endif()

message(STATUS "AC-3 PASS [7.8.4]: CMakePresets.json macos-base and linux-base no longer disable MU_ENABLE_DOTNET — .NET builds enabled on all platforms")
