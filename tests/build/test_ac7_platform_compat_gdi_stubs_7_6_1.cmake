# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
# Test AC-7: All 9 GDI/Win32 stubs present in PlatformCompat.h

cmake_minimum_required(VERSION 3.23)

get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
set(PLATFORM_COMPAT "${WORKSPACE_ROOT}/MuMain/src/source/Platform/PlatformCompat.h")

if(NOT EXISTS "${PLATFORM_COMPAT}")
  message(FATAL_ERROR "PlatformCompat.h not found")
endif()

file(READ "${PLATFORM_COMPAT}" compat_content)

# Verify actual definitions/declarations, not just string occurrences
set(REQUIRED_PATTERNS
  "#define RGB"
  "inline.*SetBkColor"
  "inline.*SetTextColor"
  "inline.*TextOut"
  "WM_PAINT"
  "WM_ERASEBKGND"
  "SB_VERT"
  "GCS_COMPSTR"
  "inline.*SetTimer"
)

foreach(pattern ${REQUIRED_PATTERNS})
  if(NOT compat_content MATCHES "${pattern}")
    message(FATAL_ERROR "AC-7 FAILED: Pattern '${pattern}' not found")
  endif()
endforeach()

message(STATUS "AC-7 PASSED")
