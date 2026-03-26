# Flow Code: VS0-QUAL-BUILD-AUDIO
# Story 7.8.1: Audio Interface Win32 Type Cleanup
# AC-STD-11: Flow code VS0-QUAL-BUILD-AUDIO traceability
#
# Verifies that:
#   1. This build test file carries the VS0-QUAL-BUILD-AUDIO flow code
#   2. The story test file header exists
#   3. The audio interface test file references the story key (7-8-1)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED TEST_BUILD_DIR)
    message(FATAL_ERROR "AC-STD-11: TEST_BUILD_DIR variable not defined")
endif()

# Check that the story's build test files exist
set(required_test_files
    "${TEST_BUILD_DIR}/test_ac1_dsplaysound_win32_guard_7_8_1.cmake"
    "${TEST_BUILD_DIR}/test_ac2_iplatformaudio_portable_types_7_8_1.cmake"
    "${TEST_BUILD_DIR}/test_ac3_miniaudiobackend_no_win32_types_7_8_1.cmake"
    "${TEST_BUILD_DIR}/test_ac5_check_win32_guards_audio_7_8_1.cmake"
)

foreach(f ${required_test_files})
    if(NOT EXISTS "${f}")
        message(FATAL_ERROR "AC-STD-11 FAILED: Required test file missing: ${f}")
    endif()

    file(READ "${f}" file_content)
    string(FIND "${file_content}" "VS0-QUAL-BUILD-AUDIO" pos_flow)
    if(pos_flow EQUAL -1)
        message(FATAL_ERROR "AC-STD-11 FAILED: Flow code VS0-QUAL-BUILD-AUDIO not found in: ${f}")
    endif()

    string(FIND "${file_content}" "7.8.1" pos_story)
    if(pos_story EQUAL -1)
        message(FATAL_ERROR "AC-STD-11 FAILED: Story reference '7.8.1' not found in: ${f}")
    endif()
endforeach()

message(STATUS "AC-STD-11 PASSED: Flow code VS0-QUAL-BUILD-AUDIO present in all 4 story 7.8.1 build tests")
