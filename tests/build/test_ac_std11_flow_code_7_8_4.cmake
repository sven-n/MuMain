# Flow Code: VS0-QUAL-BUILD-DOTNET
# Story 7.8.4: .NET Client Library Native Build
# AC-STD-11: Flow code VS0-QUAL-BUILD-DOTNET traceability
#
# Verifies that:
#   1. All story 7.8.4 build test files exist in the tests/build directory
#   2. Each file carries the VS0-QUAL-BUILD-DOTNET flow code
#   3. Each file references the story key (7.8.4)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED TEST_BUILD_DIR)
    message(FATAL_ERROR "AC-STD-11: TEST_BUILD_DIR variable not defined")
endif()

# Check that all story 7.8.4 build test files exist
set(required_test_files
    "${TEST_BUILD_DIR}/test_ac1_src_cmake_rid_detection_7_8_4.cmake"
    "${TEST_BUILD_DIR}/test_ac2_src_cmake_lib_ext_copy_7_8_4.cmake"
    "${TEST_BUILD_DIR}/test_ac3_presets_dotnet_enabled_7_8_4.cmake"
    "${TEST_BUILD_DIR}/test_ac4_resource_h_guard_7_8_4.cmake"
)

foreach(f ${required_test_files})
    if(NOT EXISTS "${f}")
        message(FATAL_ERROR "AC-STD-11 FAILED [7.8.4]: Required test file missing: ${f}")
    endif()

    file(READ "${f}" file_content)

    string(FIND "${file_content}" "VS0-QUAL-BUILD-DOTNET" pos_flow)
    if(pos_flow EQUAL -1)
        message(FATAL_ERROR "AC-STD-11 FAILED [7.8.4]: Flow code VS0-QUAL-BUILD-DOTNET not found in: ${f}")
    endif()

    string(FIND "${file_content}" "7.8.4" pos_story)
    if(pos_story EQUAL -1)
        message(FATAL_ERROR "AC-STD-11 FAILED [7.8.4]: Story reference '7.8.4' not found in: ${f}")
    endif()
endforeach()

message(STATUS "AC-STD-11 PASSED [7.8.4]: Flow code VS0-QUAL-BUILD-DOTNET present in all 4 story 7.8.4 build tests")
