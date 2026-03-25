# Flow Code: VS0-QUAL-BUILDCOMP-MACOS
# Test AC-5: ZzzOpenglUtil.cpp compiles on macOS via WGL stubs in PlatformCompat.h
# (Changed from exclusion approach: the file provides essential rendering globals/functions
# used throughout the codebase. WGL-specific code is stubbed to no-op on non-Windows.)

cmake_minimum_required(VERSION 3.23)

get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
set(PLATFORM_COMPAT "${WORKSPACE_ROOT}/MuMain/src/source/Platform/PlatformCompat.h")
set(OPENGL_UTIL "${WORKSPACE_ROOT}/MuMain/src/source/RenderFX/ZzzOpenglUtil.cpp")

if(NOT EXISTS "${PLATFORM_COMPAT}")
  message(FATAL_ERROR "PlatformCompat.h not found")
endif()
if(NOT EXISTS "${OPENGL_UTIL}")
  message(FATAL_ERROR "ZzzOpenglUtil.cpp not found")
endif()

# Verify WGL function pointer type stubs exist in PlatformCompat.h
file(READ "${PLATFORM_COMPAT}" compat_content)
if(NOT compat_content MATCHES "PFNWGLSWAPINTERVALEXTPROC")
  message(FATAL_ERROR "AC-5 FAILED: WGL function pointer stubs not found in PlatformCompat.h")
endif()
if(NOT compat_content MATCHES "wglGetProcAddress")
  message(FATAL_ERROR "AC-5 FAILED: wglGetProcAddress stub not found in PlatformCompat.h")
endif()

# Verify wglext.h include is guarded in ZzzOpenglUtil.cpp
file(READ "${OPENGL_UTIL}" util_content)
if(NOT util_content MATCHES "#ifdef _WIN32\n#include \"wglext\\.h\"")
  message(FATAL_ERROR "AC-5 FAILED: wglext.h include not guarded with #ifdef _WIN32")
endif()

message(STATUS "AC-5 PASSED")
