# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
# Test AC-6: DSwaveIO.h #include <mmsystem.h> guarded with #ifdef _WIN32

cmake_minimum_required(VERSION 3.23)

get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
set(DSWAVEIO_H "${WORKSPACE_ROOT}/MuMain/src/source/Audio/DSwaveIO.h")

if(NOT EXISTS "${DSWAVEIO_H}")
  message(FATAL_ERROR "DSwaveIO.h not found")
endif()

file(READ "${DSWAVEIO_H}" dswaveio_content)

if(NOT dswaveio_content MATCHES "#ifdef _WIN32.*#include.*mmsystem\\.h")
  message(FATAL_ERROR "AC-6 FAILED: mmsystem include not guarded")
endif()

message(STATUS "AC-6 PASSED")
