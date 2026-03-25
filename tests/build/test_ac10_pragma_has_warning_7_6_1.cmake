# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
# Test AC-10: ZzzOpenData.cpp pragma wrapped with __has_warning guard

cmake_minimum_required(VERSION 3.23)

get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
set(ZZZ_OPENDATA "${WORKSPACE_ROOT}/MuMain/src/source/Data/ZzzOpenData.cpp")

if(NOT EXISTS "${ZZZ_OPENDATA}")
  message(FATAL_ERROR "ZzzOpenData.cpp not found")
endif()

file(READ "${ZZZ_OPENDATA}" opendata_content)

if(NOT opendata_content MATCHES "__has_warning.*Wnontrivial-memcall")
  message(FATAL_ERROR "AC-10 FAILED: Pragma not wrapped with __has_warning")
endif()

message(STATUS "AC-10 PASSED")
