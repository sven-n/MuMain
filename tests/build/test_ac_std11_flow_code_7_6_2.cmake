# Flow Code: VS0-QUAL-WIN32CLEAN-STRINCLUDE
# Story 7.6.2: Win32 String Conversion and Include Guard Cleanup
# AC-STD-11: All test files for story 7.6.2 contain the flow code traceability string.

cmake_minimum_required(VERSION 3.25)

file(GLOB test_files "${CMAKE_CURRENT_LIST_DIR}/test_*_7_6_2.cmake")

foreach(test_file ${test_files})
    # Skip the flow code test itself to avoid circular check
    get_filename_component(test_name "${test_file}" NAME)
    if(test_name STREQUAL "test_ac_std11_flow_code_7_6_2.cmake")
        continue()
    endif()

    file(READ "${test_file}" test_content)

    if(NOT test_content MATCHES "VS0-QUAL-WIN32CLEAN-STRINCLUDE")
        message(FATAL_ERROR "AC-STD-11 FAILED: Flow code 'VS0-QUAL-WIN32CLEAN-STRINCLUDE' not found in ${test_name}")
    endif()
endforeach()

message(STATUS "AC-STD-11 PASSED: All story 7.6.2 test files contain flow code VS0-QUAL-WIN32CLEAN-STRINCLUDE")
