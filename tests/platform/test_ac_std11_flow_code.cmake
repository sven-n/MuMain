# Story 2.1.1: AC-STD-11 — Flow code VS1-SDL-WINDOW-CREATE in story artifacts
# GREEN PHASE: Flow code and story references verified.
#
# Validates that:
# 1. test_platform_window.cpp contains the flow code reference
# 2. Story tag [2-1-1] appears in test names

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED TEST_DIR)
    message(FATAL_ERROR "TEST_DIR must be set to the tests/platform/ directory")
endif()

set(TEST_FILE "${TEST_DIR}/test_platform_window.cpp")

if(NOT EXISTS "${TEST_FILE}")
    message(FATAL_ERROR "AC-STD-11 FAIL: test_platform_window.cpp does not exist")
endif()

file(READ "${TEST_FILE}" test_content)

# Check for story tag [2-1-1] in test names
string(FIND "${test_content}" "[2-1-1]" tag_pos)
if(tag_pos EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL: test_platform_window.cpp missing [2-1-1] tag in test names")
endif()

# Check for VS1-SDL-WINDOW-CREATE flow code
string(FIND "${test_content}" "VS1-SDL-WINDOW-CREATE" flow_code_pos)
if(flow_code_pos EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL: test_platform_window.cpp missing VS1-SDL-WINDOW-CREATE flow code")
endif()

# Check for story reference 2.1.1
string(FIND "${test_content}" "2.1.1" story_ref_pos)
if(story_ref_pos EQUAL -1)
    message(FATAL_ERROR "AC-STD-11 FAIL: test_platform_window.cpp missing story 2.1.1 reference")
endif()

message(STATUS "AC-STD-11 PASS: Flow code VS1-SDL-WINDOW-CREATE and story references present in test file")
