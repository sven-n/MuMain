# Story 7.4.1: AC-STD-11 — Flow code VS0-QUAL-CI-NATIVE traceability
#
# Story: 7.4.1 - Native Platform CI Runners
# Flow Code: VS0-QUAL-CI-NATIVE
# Story tag: [7-4-1]
#
# RED PHASE:  Test FAILS before story 7.4.1 ATDD tests are created
#             (this file does not yet exist in the build test directory).
#
# GREEN PHASE: Test PASSES once the ATDD test files for story 7.4.1 are created
#              with correct flow code and story references.
#
# Validates:
#   AC-STD-11 — Story tag [7-4-1] appears in at least one test file
#   AC-STD-11 — Flow code VS0-QUAL-CI-NATIVE appears in at least one test file
#   AC-STD-1  — ci.yml follows concurrency group pattern (consistent with existing jobs)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CI_WORKFLOW_FILE)
    message(FATAL_ERROR "AC-STD-11: CI_WORKFLOW_FILE variable not defined")
endif()

if(NOT DEFINED TEST_BUILD_DIR)
    message(FATAL_ERROR "AC-STD-11: TEST_BUILD_DIR variable not defined")
endif()

# --- Check 1: Flow code VS0-QUAL-CI-NATIVE in this test file ---
set(_this_file "${TEST_BUILD_DIR}/test_ac_std11_flow_code_7_4_1.cmake")
if(NOT EXISTS "${_this_file}")
    message(FATAL_ERROR "AC-STD-11 FAIL [7-4-1]: test_ac_std11_flow_code_7_4_1.cmake does not exist at '${_this_file}'")
endif()

file(READ "${_this_file}" _self_content)

string(FIND "${_self_content}" "VS0-QUAL-CI-NATIVE" _flow_pos)
if(_flow_pos EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL [7-4-1]: VS0-QUAL-CI-NATIVE flow code missing from test_ac_std11_flow_code_7_4_1.cmake")
endif()

string(FIND "${_self_content}" "[7-4-1]" _tag_pos)
if(_tag_pos EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL [7-4-1]: [7-4-1] story tag missing from test_ac_std11_flow_code_7_4_1.cmake")
endif()

# --- Check 2: ci.yml has concurrency group (AC-STD-1 pattern consistency) ---
if(NOT EXISTS "${CI_WORKFLOW_FILE}")
    message(FATAL_ERROR "AC-STD-11: ci.yml does not exist: ${CI_WORKFLOW_FILE}")
endif()

file(READ "${CI_WORKFLOW_FILE}" CI_CONTENT)

string(FIND "${CI_CONTENT}" "concurrency:" _pos_concurrency)
if(_pos_concurrency EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL [7-4-1]: 'concurrency:' group missing from ci.yml — AC-STD-1 pattern consistency violated")
endif()

string(FIND "${CI_CONTENT}" "cancel-in-progress:" _pos_cancel)
if(_pos_cancel EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL [7-4-1]: 'cancel-in-progress:' missing from ci.yml concurrency block")
endif()

# --- Check 3: ci.yml job naming follows existing pattern (name: ... Present) ---
string(FIND "${CI_CONTENT}" "macOS Native Build" _pos_macos_name)
if(_pos_macos_name EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL [7-4-1]: 'macOS Native Build' job name not found — AC-STD-1 step naming convention not followed")
endif()

string(FIND "${CI_CONTENT}" "Linux Native Build" _pos_linux_name)
if(_pos_linux_name EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL [7-4-1]: 'Linux Native Build' job name not found — AC-STD-1 step naming convention not followed")
endif()

message(STATUS "AC-STD-11 PASS [7-4-1]: Flow code VS0-QUAL-CI-NATIVE and story tag [7-4-1] present; ci.yml follows existing patterns")
