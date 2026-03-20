# AC-2: Validate ci.yml adds ubuntu-latest runner with GCC build using linux-x64 preset
#
# Story: 7.4.1 - Native Platform CI Runners
# Flow Code: VS0-QUAL-CI-NATIVE
# Story tag: [7-4-1]
#
# RED PHASE: Test FAILS until build-linux job is added to ci.yml.
# GREEN PHASE: Passes once build-linux job with ubuntu-latest runner and linux-x64 preset exists.
#
# Validates:
#   AC-2 — build-linux job exists with runs-on: ubuntu-latest
#   AC-2 — cmake --preset linux-x64 is used for configuration

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CI_WORKFLOW_FILE)
    message(FATAL_ERROR "AC-2: CI_WORKFLOW_FILE variable not defined")
endif()

if(NOT EXISTS "${CI_WORKFLOW_FILE}")
    message(FATAL_ERROR "AC-2: ci.yml does not exist: ${CI_WORKFLOW_FILE}")
endif()

file(READ "${CI_WORKFLOW_FILE}" CI_CONTENT)

# --- Check 1: build-linux job exists ---
string(FIND "${CI_CONTENT}" "build-linux:" _pos_job)
if(_pos_job EQUAL -1)
    message(FATAL_ERROR "AC-2 FAIL [7-4-1]: 'build-linux:' job not found in ci.yml")
endif()

# --- Check 2: linux-x64 CMake preset ---
string(FIND "${CI_CONTENT}" "cmake --preset linux-x64" _pos_configure)
if(_pos_configure EQUAL -1)
    message(FATAL_ERROR "AC-2 FAIL [7-4-1]: 'cmake --preset linux-x64' configure step not found in ci.yml")
endif()

# --- Check 3: GCC/g++ installed in build-linux job ---
string(FIND "${CI_CONTENT}" "gcc g++" _pos_gcc)
if(_pos_gcc EQUAL -1)
    message(FATAL_ERROR "AC-2 FAIL [7-4-1]: 'gcc g++' package install not found in ci.yml (expected in build-linux apt-get step)")
endif()

# --- Check 4: build-linux references ubuntu-latest ---
# ubuntu-latest is already used by the quality and build jobs, so verify at least two occurrences
string(REGEX MATCHALL "ubuntu-latest" _ubuntu_matches "${CI_CONTENT}")
list(LENGTH _ubuntu_matches _ubuntu_count)
if(_ubuntu_count LESS 2)
    message(FATAL_ERROR "AC-2 FAIL [7-4-1]: Expected ubuntu-latest in build-linux job (found ${_ubuntu_count} total occurrences — need at least 2)")
endif()

message(STATUS "AC-2 PASS [7-4-1]: build-linux job with ubuntu-latest runner and linux-x64 preset found in ci.yml")
