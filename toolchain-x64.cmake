# CMake toolchain file for x64 (64-bit) builds
# Cross-platform: works on Windows, Linux, and macOS

# Force 64-bit pointer size BEFORE compiler detection
set(CMAKE_SIZEOF_VOID_P 8 CACHE INTERNAL "Pointer size in bytes" FORCE)

# Platform-specific configuration
if(WIN32)
  # On Windows with MSVC, rely on vcvarsall.bat environment setup
  # Visual Studio and Ninja will use the environment provided by the developer command prompt
  set(CMAKE_SYSTEM_NAME Windows)
  set(CMAKE_SYSTEM_PROCESSOR AMD64)
  message(STATUS "x64 Toolchain: Configured for 64-bit Windows build")

elseif(UNIX AND NOT APPLE)
  # On Linux, add -m64 flag to force 64-bit compilation (usually default)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64" CACHE STRING "C compiler flags" FORCE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64" CACHE STRING "C++ compiler flags" FORCE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64" CACHE STRING "Linker flags" FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m64" CACHE STRING "Shared linker flags" FORCE)
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -m64" CACHE STRING "Module linker flags" FORCE)
  message(STATUS "x64 Toolchain: Configured for 64-bit Linux build")

elseif(APPLE)
  # On macOS, add -m64 flag (default on modern macOS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64" CACHE STRING "C compiler flags" FORCE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64" CACHE STRING "C++ compiler flags" FORCE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64" CACHE STRING "Linker flags" FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m64" CACHE STRING "Shared linker flags" FORCE)
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -m64" CACHE STRING "Module linker flags" FORCE)
  message(STATUS "x64 Toolchain: Configured for 64-bit macOS build")
endif()
