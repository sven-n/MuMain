# Story 7.1.1: AC-STD-11 — Flow code VS0-QUAL-ERRORREPORT-XPLAT traceability
#
# RED PHASE:  Test FAILS before story 7.1.1 implementation starts
#             (test file does not yet exist).
#
# GREEN PHASE: Test PASSES once test_error_report.cpp is created with
#              flow code and story references.
#
# Validates:
#   AC-STD-11 — commit/test references contain VS0-QUAL-ERRORREPORT-XPLAT
#   Story tag [7-1-1] appears in test names
#   Story reference 7.1.1 appears in test file

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED TEST_DIR)
    message(FATAL_ERROR "TEST_DIR must be set to the tests/core/ directory")
endif()

set(TEST_FILE "${TEST_DIR}/test_error_report.cpp")

if(NOT EXISTS "${TEST_FILE}")
    message(FATAL_ERROR
        "AC-STD-11 FAIL: test_error_report.cpp does not exist at '${TEST_FILE}'.\n"
        "Create the file as part of story 7.1.1 Task 5.")
endif()

file(READ "${TEST_FILE}" test_content)

# Check for story tag [7-1-1] in test names
string(FIND "${test_content}" "[7-1-1]" tag_pos)
if(tag_pos EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL: test_error_report.cpp missing [7-1-1] tag in TEST_CASE names")
endif()

# Check for VS0-QUAL-ERRORREPORT-XPLAT flow code
string(FIND "${test_content}" "VS0-QUAL-ERRORREPORT-XPLAT" flow_code_pos)
if(flow_code_pos EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL: test_error_report.cpp missing VS0-QUAL-ERRORREPORT-XPLAT flow code")
endif()

# Check for story reference 7.1.1
string(FIND "${test_content}" "7.1.1" story_ref_pos)
if(story_ref_pos EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL: test_error_report.cpp missing story 7.1.1 reference")
endif()

message(STATUS "AC-STD-11 PASS: Flow code VS0-QUAL-ERRORREPORT-XPLAT and story references present in test_error_report.cpp")
