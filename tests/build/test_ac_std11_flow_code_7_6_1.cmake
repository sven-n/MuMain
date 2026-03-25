# Story 7.6.1: AC-STD-11 — Flow code VS0-QUAL-BUILDCOMP-MACOS traceability
# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
#
# RED PHASE:  Test FAILS if the story 7.6.1 flow code is absent from test files.
#             AC-STD-11 requires the flow code VS0-QUAL-BUILDCOMP-MACOS in commit messages
#             and in test file headers for traceability.
#
# GREEN PHASE: Test PASSES when:
#   - At least one test file in tests/build/ contains VS0-QUAL-BUILDCOMP-MACOS
#   - The flow code matches the story's designated flow code
#
# Validates:
#   AC-STD-11 — Flow code VS0-QUAL-BUILDCOMP-MACOS present in story 7.6.1 test files

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED TEST_BUILD_DIR)
    message(FATAL_ERROR "TEST_BUILD_DIR must be set to the tests/build/ directory")
endif()

if(NOT IS_DIRECTORY "${TEST_BUILD_DIR}")
    message(FATAL_ERROR "AC-STD-11 FAIL: tests/build/ directory not found at '${TEST_BUILD_DIR}'")
endif()

set(FLOW_CODE "VS0-QUAL-BUILDCOMP-MACOS")
set(STORY_KEY "7_6_1")

# Find test files for this story
file(GLOB story_test_files "${TEST_BUILD_DIR}/*${STORY_KEY}*.cmake")

if(NOT story_test_files)
    message(FATAL_ERROR
        "AC-STD-11 FAIL: No test files matching '*${STORY_KEY}*.cmake' found in ${TEST_BUILD_DIR}.\n"
        "Story 7.6.1 requires test files with the flow code ${FLOW_CODE}.\n"
        "Fix: Create test files named test_ac*_*_7_6_1.cmake in tests/build/")
endif()

set(found_flow_code FALSE)

foreach(test_file IN LISTS story_test_files)
    file(READ "${test_file}" test_content)
    string(FIND "${test_content}" "${FLOW_CODE}" flow_pos)
    if(NOT flow_pos EQUAL -1)
        set(found_flow_code TRUE)
        break()
    endif()
endforeach()

if(NOT found_flow_code)
    message(FATAL_ERROR
        "AC-STD-11 FAIL: Flow code '${FLOW_CODE}' not found in any story 7.6.1 test file.\n"
        "Fix: Add '# Flow Code: ${FLOW_CODE}' header comment to story 7.6.1 test files.")
endif()

list(LENGTH story_test_files test_count)
message(STATUS "AC-STD-11 PASS: Flow code ${FLOW_CODE} found in story 7.6.1 test files (${test_count} files)")
