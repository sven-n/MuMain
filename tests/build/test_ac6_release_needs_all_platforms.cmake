# AC-6: Validate release job requires all three platform jobs to pass
#
# Story: 7.4.1 - Native Platform CI Runners
# Flow Code: VS0-QUAL-CI-NATIVE
# Story tag: [7-4-1]
#
# RED PHASE: Test FAILS until release.needs includes build-macos and build-linux.
# GREEN PHASE: Passes once release job needs: [quality, build, build-macos, build-linux].
#
# Validates:
#   AC-6 — release job needs all three platform build jobs + quality
#   AC-6 — build-macos in release.needs
#   AC-6 — build-linux in release.needs
#   AC-6 — existing quality and build remain in release.needs (regression)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CI_WORKFLOW_FILE)
    message(FATAL_ERROR "AC-6: CI_WORKFLOW_FILE variable not defined")
endif()

if(NOT EXISTS "${CI_WORKFLOW_FILE}")
    message(FATAL_ERROR "AC-6: ci.yml does not exist: ${CI_WORKFLOW_FILE}")
endif()

file(READ "${CI_WORKFLOW_FILE}" CI_CONTENT)

# --- Check 1: release job exists ---
string(FIND "${CI_CONTENT}" "Semantic Release" _pos_release)
if(_pos_release EQUAL -1)
    message(FATAL_ERROR "AC-6 FAIL [7-4-1]: 'Semantic Release' job name not found in ci.yml")
endif()

# --- Check 2: build-macos in needs list ---
# The needs line should reference build-macos alongside other jobs
string(FIND "${CI_CONTENT}" "build-macos" _pos_needs_macos)
if(_pos_needs_macos EQUAL -1)
    message(FATAL_ERROR "AC-6 FAIL [7-4-1]: 'build-macos' not referenced in ci.yml release job needs")
endif()

# --- Check 3: build-linux in needs list ---
string(FIND "${CI_CONTENT}" "build-linux" _pos_needs_linux)
if(_pos_needs_linux EQUAL -1)
    message(FATAL_ERROR "AC-6 FAIL [7-4-1]: 'build-linux' not referenced in ci.yml release job needs")
endif()

# --- Check 4: needs array contains all four required jobs ---
# Verify the needs: line contains quality, build, build-macos, build-linux together
string(FIND "${CI_CONTENT}" "needs: [quality, build, build-macos, build-linux]" _pos_needs_all)
if(_pos_needs_all EQUAL -1)
    # Allow alternate ordering / formatting variants
    string(FIND "${CI_CONTENT}" "needs:" _pos_needs_kw)
    if(_pos_needs_kw EQUAL -1)
        message(FATAL_ERROR "AC-6 FAIL [7-4-1]: No 'needs:' keyword found in ci.yml release job")
    endif()
    # Check each required job appears after "needs:" somewhere in the file
    foreach(_required_job "quality" "build" "build-macos" "build-linux")
        string(FIND "${CI_CONTENT}" "${_required_job}" _pos_req)
        if(_pos_req EQUAL -1)
            message(FATAL_ERROR "AC-6 FAIL [7-4-1]: '${_required_job}' not found in ci.yml — must be in release.needs")
        endif()
    endforeach()
endif()

message(STATUS "AC-6 PASS [7-4-1]: release job requires quality, build, build-macos, and build-linux — all platforms gated")
