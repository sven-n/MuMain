# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
# Test AC-8: xstreambuf.cpp no untyped delete void*

cmake_minimum_required(VERSION 3.23)

get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
set(XSTREAMBUF "${WORKSPACE_ROOT}/MuMain/src/source/ThirdParty/xstreambuf.cpp")

if(NOT EXISTS "${XSTREAMBUF}")
  message(FATAL_ERROR "xstreambuf.cpp not found")
endif()

file(READ "${XSTREAMBUF}" xstream_content)

if(xstream_content MATCHES "delete\\s+m_pBuffer\\s*[;)]")
  message(FATAL_ERROR "AC-8 FAILED: Untyped delete void* found")
endif()

string(REGEX MATCHALL "delete\\[\\].*cast<char\\*>\\(m_pBuffer\\)" matches "${xstream_content}")
list(LENGTH matches match_count)

if(match_count LESS 3)
  message(FATAL_ERROR "AC-8 FAILED: Expected 3+ casted deletes, found ${match_count}")
endif()

message(STATUS "AC-8 PASSED")
