# Flow Code: VS0-QUAL-BUILD-HEADERS
# Story 7.8.2: Gameplay Header Cross-Platform Fixes
# AC-STD-11: Flow code VS0-QUAL-BUILD-HEADERS traceability
#
# Verifies that:
#   1. All story 7.8.2 build test files exist in the tests/build directory
#   2. Each file carries the VS0-QUAL-BUILD-HEADERS flow code
#   3. Each file references the story key (7.8.2)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED TEST_BUILD_DIR)
    message(FATAL_ERROR "AC-STD-11: TEST_BUILD_DIR variable not defined")
endif()

# Check that all story 7.8.2 build test files exist
set(required_test_files
    "${TEST_BUILD_DIR}/test_ac1_mu_enum_inline_7_8_2.cmake"
    "${TEST_BUILD_DIR}/test_ac2_zzzpath_errorreport_include_7_8_2.cmake"
    "${TEST_BUILD_DIR}/test_ac3_skillstructs_multilanguage_7_8_2.cmake"
    "${TEST_BUILD_DIR}/test_ac4_csitemoption_type_includes_7_8_2.cmake"
)

foreach(f ${required_test_files})
    if(NOT EXISTS "${f}")
        message(FATAL_ERROR "AC-STD-11 FAILED: Required test file missing: ${f}")
    endif()

    file(READ "${f}" file_content)

    string(FIND "${file_content}" "VS0-QUAL-BUILD-HEADERS" pos_flow)
    if(pos_flow EQUAL -1)
        message(FATAL_ERROR "AC-STD-11 FAILED: Flow code VS0-QUAL-BUILD-HEADERS not found in: ${f}")
    endif()

    string(FIND "${file_content}" "7.8.2" pos_story)
    if(pos_story EQUAL -1)
        message(FATAL_ERROR "AC-STD-11 FAILED: Story reference '7.8.2' not found in: ${f}")
    endif()
endforeach()

message(STATUS "AC-STD-11 PASSED: Flow code VS0-QUAL-BUILD-HEADERS present in all 4 story 7.8.2 build tests")
