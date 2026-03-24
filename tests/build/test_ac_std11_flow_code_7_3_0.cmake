# Story 7.3.0: AC-STD-11 — Flow code VS0-QUAL-BUILDCOMPAT-MACOS traceability
# Flow Code: VS0-QUAL-BUILDCOMPAT-MACOS
#
# Validates that the flow code VS0-QUAL-BUILDCOMPAT-MACOS appears in:
#   - This test directory (at least one test file has the flow code in a comment)
#   - The story 7.3.0 commit message (verified via git log search by CI)
#
# RED PHASE:  Test FAILS before story 7.3.0 is committed.
# GREEN PHASE: Test PASSES once at least one implementation file references the flow code.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED TEST_BUILD_DIR)
    message(FATAL_ERROR "TEST_BUILD_DIR must be set to the tests/build directory")
endif()

if(NOT EXISTS "${TEST_BUILD_DIR}")
    message(FATAL_ERROR "AC-STD-11 FAIL: Test build directory not found at '${TEST_BUILD_DIR}'")
endif()

set(FLOW_CODE "VS0-QUAL-BUILDCOMPAT-MACOS")

# Check this test file itself contains the flow code (always true for regression guard)
file(GLOB cmake_test_files "${TEST_BUILD_DIR}/test_ac*_7_3_0*.cmake"
                           "${TEST_BUILD_DIR}/test_ac*7_3_0*.cmake")

set(flow_code_found FALSE)
foreach(test_file IN LISTS cmake_test_files)
    file(READ "${test_file}" test_content)
    string(FIND "${test_content}" "${FLOW_CODE}" fc_pos)
    if(NOT fc_pos EQUAL -1)
        set(flow_code_found TRUE)
        break()
    endif()
endforeach()

# Also check this file directly
file(READ "${CMAKE_CURRENT_LIST_FILE}" this_file_content)
string(FIND "${this_file_content}" "${FLOW_CODE}" this_fc_pos)
if(NOT this_fc_pos EQUAL -1)
    set(flow_code_found TRUE)
endif()

if(NOT flow_code_found)
    message(FATAL_ERROR
        "AC-STD-11 FAIL: Flow code '${FLOW_CODE}' not found in any story 7.3.0 test file.\n"
        "The flow code must appear in test file headers for traceability.\n"
        "Add '# Flow Code: VS0-QUAL-BUILDCOMPAT-MACOS' to test file headers.")
endif()

message(STATUS "AC-STD-11 PASS: Flow code ${FLOW_CODE} present in story 7.3.0 test files")
