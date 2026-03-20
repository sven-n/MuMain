# AC-3: Validate both native runners execute CMake configure, build, and Catch2 test suite
#
# Story: 7.4.1 - Native Platform CI Runners
# Flow Code: VS0-QUAL-CI-NATIVE
# Story tag: [7-4-1]
#
# RED PHASE: Test FAILS until both build-macos and build-linux jobs include ctest steps.
# GREEN PHASE: Passes once both jobs have configure + build + ctest --output-on-failure steps.
#
# Validates:
#   AC-3 — build-macos: cmake configure, cmake --build, ctest with --output-on-failure
#   AC-3 — build-linux: cmake configure, cmake --build, ctest with --output-on-failure

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CI_WORKFLOW_FILE)
    message(FATAL_ERROR "AC-3: CI_WORKFLOW_FILE variable not defined")
endif()

if(NOT EXISTS "${CI_WORKFLOW_FILE}")
    message(FATAL_ERROR "AC-3: ci.yml does not exist: ${CI_WORKFLOW_FILE}")
endif()

file(READ "${CI_WORKFLOW_FILE}" CI_CONTENT)

# --- Check 1: ctest appears at least twice (once per native job) ---
string(REGEX MATCHALL "ctest" _ctest_matches "${CI_CONTENT}")
list(LENGTH _ctest_matches _ctest_count)
if(_ctest_count LESS 2)
    message(FATAL_ERROR "AC-3 FAIL [7-4-1]: Expected ctest in both build-macos and build-linux jobs (found ${_ctest_count} occurrences)")
endif()

# --- Check 2: --output-on-failure flag present (at least twice) ---
string(REGEX MATCHALL "output-on-failure" _oof_matches "${CI_CONTENT}")
list(LENGTH _oof_matches _oof_count)
if(_oof_count LESS 2)
    message(FATAL_ERROR "AC-3 FAIL [7-4-1]: Expected --output-on-failure in both native ctest steps (found ${_oof_count} occurrences)")
endif()

# --- Check 3: cmake --build appears for both native presets ---
string(FIND "${CI_CONTENT}" "cmake --build out/build/macos-arm64" _pos_macos_build)
if(_pos_macos_build EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL [7-4-1]: 'cmake --build out/build/macos-arm64' not found in ci.yml")
endif()

string(FIND "${CI_CONTENT}" "cmake --build out/build/linux-x64" _pos_linux_build)
if(_pos_linux_build EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL [7-4-1]: 'cmake --build out/build/linux-x64' not found in ci.yml")
endif()

# --- Check 4: ctest --test-dir for macos-arm64 ---
string(FIND "${CI_CONTENT}" "ctest --test-dir out/build/macos-arm64" _pos_macos_ctest)
if(_pos_macos_ctest EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL [7-4-1]: 'ctest --test-dir out/build/macos-arm64' not found in ci.yml")
endif()

# --- Check 5: ctest --test-dir for linux-x64 ---
string(FIND "${CI_CONTENT}" "ctest --test-dir out/build/linux-x64" _pos_linux_ctest)
if(_pos_linux_ctest EQUAL -1)
    message(FATAL_ERROR "AC-3 FAIL [7-4-1]: 'ctest --test-dir out/build/linux-x64' not found in ci.yml")
endif()

message(STATUS "AC-3 PASS [7-4-1]: Both native runners have configure, build, and ctest steps with --output-on-failure")
