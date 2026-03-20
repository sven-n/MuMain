# AC-1: Validate ci.yml adds macos-latest runner with Clang build using macos-arm64 preset
#
# Story: 7.4.1 - Native Platform CI Runners
# Flow Code: VS0-QUAL-CI-NATIVE
# Story tag: [7-4-1]
#
# RED PHASE: Test FAILS until build-macos job is added to ci.yml.
# GREEN PHASE: Passes once build-macos job with macos-latest runner and macos-arm64 preset exists.
#
# Validates:
#   AC-1 — build-macos job exists with runs-on: macos-latest
#   AC-1 — cmake --preset macos-arm64 is used for configuration

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CI_WORKFLOW_FILE)
    message(FATAL_ERROR "AC-1: CI_WORKFLOW_FILE variable not defined")
endif()

if(NOT EXISTS "${CI_WORKFLOW_FILE}")
    message(FATAL_ERROR "AC-1: ci.yml does not exist: ${CI_WORKFLOW_FILE}")
endif()

file(READ "${CI_WORKFLOW_FILE}" CI_CONTENT)

# --- Check 1: build-macos job exists ---
string(FIND "${CI_CONTENT}" "build-macos:" _pos_job)
if(_pos_job EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7-4-1]: 'build-macos:' job not found in ci.yml")
endif()

# --- Check 2: macos-latest runner ---
string(FIND "${CI_CONTENT}" "macos-latest" _pos_runner)
if(_pos_runner EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7-4-1]: 'macos-latest' runner not found in ci.yml")
endif()

# --- Check 3: macos-arm64 CMake preset ---
string(FIND "${CI_CONTENT}" "macos-arm64" _pos_preset)
if(_pos_preset EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7-4-1]: '--preset macos-arm64' or 'macos-arm64' not found in ci.yml")
endif()

# --- Check 4: cmake configure step references the preset ---
string(FIND "${CI_CONTENT}" "cmake --preset macos-arm64" _pos_configure)
if(_pos_configure EQUAL -1)
    message(FATAL_ERROR "AC-1 FAIL [7-4-1]: 'cmake --preset macos-arm64' configure step not found in ci.yml")
endif()

message(STATUS "AC-1 PASS [7-4-1]: build-macos job with macos-latest runner and macos-arm64 preset found in ci.yml")
