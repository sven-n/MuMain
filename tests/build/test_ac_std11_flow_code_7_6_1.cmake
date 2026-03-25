# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
# Test AC-STD-11: Flow code present in all test files

cmake_minimum_required(VERSION 3.23)

file(GLOB test_files "${CMAKE_CURRENT_LIST_DIR}/test_*_7_6_1.cmake")

foreach(test_file ${test_files})
  file(READ "${test_file}" test_content)
  
  if(NOT test_content MATCHES "VS0-QUAL-BUILDCOMP-MACOS")
    message(FATAL_ERROR "AC-STD-11 FAILED: Flow code not found")
  endif()
endforeach()

message(STATUS "AC-STD-11 PASSED")
