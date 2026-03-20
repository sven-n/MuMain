# AC-5: Validate existing MinGW cross-compile job remains unchanged
#
# Story: 7.4.1 - Native Platform CI Runners
# Flow Code: VS0-QUAL-CI-NATIVE
# Story tag: [7-4-1]
#
# This test is a regression safety guard — it passes if the MinGW build job
# retains its essential structure after native runner jobs are added.
#
# GREEN PHASE from the start: MinGW job was already present and should not change.
#
# Validates:
#   AC-5 — 'build:' job with MinGW toolchain exists and is unchanged
#   AC-5 — mingw-w64-i686.cmake toolchain reference intact
#   AC-5 — artifact upload step still present
#   AC-5 — Main.exe artifact path intact

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CI_WORKFLOW_FILE)
    message(FATAL_ERROR "AC-5: CI_WORKFLOW_FILE variable not defined")
endif()

if(NOT EXISTS "${CI_WORKFLOW_FILE}")
    message(FATAL_ERROR "AC-5: ci.yml does not exist: ${CI_WORKFLOW_FILE}")
endif()

file(READ "${CI_WORKFLOW_FILE}" CI_CONTENT)

# --- Check 1: MinGW 'build:' job exists ---
string(FIND "${CI_CONTENT}" "MinGW Build" _pos_job_name)
if(_pos_job_name EQUAL -1)
    message(FATAL_ERROR "AC-5 FAIL [7-4-1]: 'MinGW Build' job name not found in ci.yml — MinGW job may have been removed or renamed")
endif()

# --- Check 2: mingw-w64-i686 toolchain file referenced ---
string(FIND "${CI_CONTENT}" "mingw-w64-i686.cmake" _pos_toolchain)
if(_pos_toolchain EQUAL -1)
    message(FATAL_ERROR "AC-5 FAIL [7-4-1]: mingw-w64-i686.cmake toolchain reference missing from ci.yml")
endif()

# --- Check 3: MinGW cross-compile packages still installed ---
string(FIND "${CI_CONTENT}" "mingw-w64 g++-mingw-w64-i686" _pos_mingw_packages)
if(_pos_mingw_packages EQUAL -1)
    message(FATAL_ERROR "AC-5 FAIL [7-4-1]: mingw-w64 package install step missing from ci.yml")
endif()

# --- Check 4: Artifact upload for Main.exe still present ---
string(FIND "${CI_CONTENT}" "Main.exe" _pos_artifact)
if(_pos_artifact EQUAL -1)
    message(FATAL_ERROR "AC-5 FAIL [7-4-1]: Main.exe artifact reference missing — MinGW artifact upload step may have been removed")
endif()

# --- Check 5: upload-artifact step still present ---
string(FIND "${CI_CONTENT}" "upload-artifact" _pos_upload)
if(_pos_upload EQUAL -1)
    message(FATAL_ERROR "AC-5 FAIL [7-4-1]: actions/upload-artifact step missing from ci.yml")
endif()

# --- Check 6: build-mingw directory still used ---
string(FIND "${CI_CONTENT}" "build-mingw" _pos_build_mingw)
if(_pos_build_mingw EQUAL -1)
    message(FATAL_ERROR "AC-5 FAIL [7-4-1]: 'build-mingw' build directory reference missing from ci.yml")
endif()

message(STATUS "AC-5 PASS [7-4-1]: MinGW cross-compile job is intact — toolchain, packages, and artifact upload unchanged")
