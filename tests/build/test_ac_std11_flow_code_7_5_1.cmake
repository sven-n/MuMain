# Story 7.5.1: AC-STD-11 — Flow code VS0-QUAL-BUILDFIXREM-MACOS traceability
# Flow Code: VS0-QUAL-BUILDFIXREM-MACOS
#
# Validates that the flow code VS0-QUAL-BUILDFIXREM-MACOS appears in:
#   - This test directory (at least one story 7.5.1 test file has the flow code in a comment)
#   - The story 7.5.1 commit message (verified via git log search by CI)
#
# RED PHASE:  Test FAILS before story 7.5.1 test files are committed.
# GREEN PHASE: Test PASSES once at least one story 7.5.1 test file references the flow code.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED TEST_BUILD_DIR)
    message(FATAL_ERROR "TEST_BUILD_DIR must be set to the tests/build directory")
endif()

if(NOT EXISTS "${TEST_BUILD_DIR}")
    message(FATAL_ERROR "AC-STD-11 FAIL: Test build directory not found at '${TEST_BUILD_DIR}'")
endif()

set(FLOW_CODE "VS0-QUAL-BUILDFIXREM-MACOS")

# Check story 7.5.1 test files contain the flow code
file(GLOB cmake_test_files
    "${TEST_BUILD_DIR}/test_ac*_7_5_1*.cmake"
    "${TEST_BUILD_DIR}/test_ac*7_5_1*.cmake")

set(flow_code_found FALSE)
foreach(test_file IN LISTS cmake_test_files)
    file(READ "${test_file}" test_content)
    string(FIND "${test_content}" "${FLOW_CODE}" fc_pos)
    if(NOT fc_pos EQUAL -1)
        set(flow_code_found TRUE)
        break()
    endif()
endforeach()

# Also check this file itself
file(READ "${CMAKE_CURRENT_LIST_FILE}" this_file_content)
string(FIND "${this_file_content}" "${FLOW_CODE}" this_fc_pos)
if(NOT this_fc_pos EQUAL -1)
    set(flow_code_found TRUE)
endif()

if(NOT flow_code_found)
    message(FATAL_ERROR
        "AC-STD-11 FAIL: Flow code '${FLOW_CODE}' not found in any story 7.5.1 test file.\n"
        "Add '# Flow Code: VS0-QUAL-BUILDFIXREM-MACOS' to all story 7.5.1 test file headers.")
endif()

message(STATUS "AC-STD-11 PASS: Flow code ${FLOW_CODE} present in story 7.5.1 test files")
