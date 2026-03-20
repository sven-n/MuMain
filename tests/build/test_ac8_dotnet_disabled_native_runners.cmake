# AC-8: Validate .NET SDK is NOT required on native runners (MU_ENABLE_DOTNET=OFF)
#
# Story: 7.4.1 - Native Platform CI Runners
# Flow Code: VS0-QUAL-CI-NATIVE
# Story tag: [7-4-1]
#
# RED PHASE: Test FAILS until native presets explicitly set MU_ENABLE_DOTNET=OFF.
# GREEN PHASE: Passes once macos-arm64 and linux-x64 presets disable .NET and
#              native CI jobs omit dotnet setup/invocation.
#
# Validates:
#   AC-8 — No 'setup-dotnet' or 'dotnet build' step in native runner jobs
#   AC-8 — Native presets (macos-base, linux-base) set MU_ENABLE_DOTNET=OFF

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CI_WORKFLOW_FILE)
    message(FATAL_ERROR "AC-8: CI_WORKFLOW_FILE variable not defined")
endif()

if(NOT EXISTS "${CI_WORKFLOW_FILE}")
    message(FATAL_ERROR "AC-8: ci.yml does not exist: ${CI_WORKFLOW_FILE}")
endif()

file(READ "${CI_WORKFLOW_FILE}" CI_CONTENT)

# --- Check 1: No dotnet setup action in ci.yml for native jobs ---
string(FIND "${CI_CONTENT}" "dotnet/setup-dotnet" _pos_setup_dotnet)
if(NOT _pos_setup_dotnet EQUAL -1)
    message(FATAL_ERROR "AC-8 FAIL [7-4-1]: 'dotnet/setup-dotnet' action found in ci.yml — native runners must not require .NET SDK")
endif()

# --- Check 2: Native presets explicitly set MU_ENABLE_DOTNET=OFF ---
# Validate in CMakePresets.json (the source of truth), not ci.yml.
get_filename_component(_ci_dir "${CI_WORKFLOW_FILE}" DIRECTORY)
set(_presets_file "${_ci_dir}/../../CMakePresets.json")
get_filename_component(_presets_file "${_presets_file}" ABSOLUTE)

if(NOT EXISTS "${_presets_file}")
    message(FATAL_ERROR "AC-8 FAIL [7-4-1]: CMakePresets.json not found at ${_presets_file}")
endif()

file(READ "${_presets_file}" PRESETS_CONTENT)

string(FIND "${PRESETS_CONTENT}" "\"MU_ENABLE_DOTNET\": \"OFF\"" _pos_dotnet_preset)
if(_pos_dotnet_preset EQUAL -1)
    message(FATAL_ERROR "AC-8 FAIL [7-4-1]: Native presets do not set MU_ENABLE_DOTNET=OFF in CMakePresets.json")
endif()

# --- Check 3: Native CI jobs do not invoke 'dotnet' command directly ---
string(FIND "${CI_CONTENT}" "run: dotnet" _pos_dotnet_run)
if(NOT _pos_dotnet_run EQUAL -1)
    message(FATAL_ERROR "AC-8 FAIL [7-4-1]: 'run: dotnet' command found in ci.yml — native runners must not invoke dotnet directly")
endif()

message(STATUS "AC-8 PASS [7-4-1]: No dotnet setup or invocation in native runner jobs; MU_ENABLE_DOTNET=OFF set in native presets")
