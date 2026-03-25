# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
# Test AC-4: quality_gate in .pcc-config.yaml includes cmake --build build

cmake_minimum_required(VERSION 3.23)

get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
set(PCC_CONFIG "${WORKSPACE_ROOT}/.pcc-config.yaml")

if(NOT EXISTS "${PCC_CONFIG}")
  message(FATAL_ERROR "PCC config not found")
endif()

file(READ "${PCC_CONFIG}" pcc_content)

if(NOT pcc_content MATCHES "quality_gate.*cmake.*--build.*build")
  message(FATAL_ERROR "AC-4 FAILED: quality_gate does not include build step")
endif()

message(STATUS "AC-4 PASSED")
