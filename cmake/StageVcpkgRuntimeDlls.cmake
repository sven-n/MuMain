# Copies every DLL that EXE loads, directly or through another DLL (libcurl ->
# zlib), from vcpkg's bin directory next to EXE.
#
# Usage: cmake -DEXE=<Main.exe> -DDLL_DIR=<vcpkg bin dir> -DDUMPBIN=<dumpbin.exe>
#              -P StageVcpkgRuntimeDlls.cmake
#
# vcpkg's own applocal step does the same, but only when the vcpkg toolchain
# was loaded by the configure that generated the build. A CMakeSystem.cmake
# cached from a configure without it silently drops that step.

foreach(var EXE DLL_DIR DUMPBIN)
  if(NOT ${var})
    message(FATAL_ERROR "${var} must be defined")
  endif()
endforeach()

if(NOT IS_DIRECTORY "${DLL_DIR}")
  # Static triplets have no bin directory, so this isn't an error.
  message(STATUS "No vcpkg DLL directory at ${DLL_DIR}; nothing to stage")
  return()
endif()

if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()

set(CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM "windows+pe")
set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL "dumpbin")
set(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND "${DUMPBIN}")

file(GET_RUNTIME_DEPENDENCIES
  EXECUTABLES "${EXE}"
  DIRECTORIES "${DLL_DIR}"
  PRE_EXCLUDE_REGEXES "^api-ms-" "^ext-ms-"
  # Don't walk into the OS DLLs: that runs dumpbin on hundreds of files, and
  # their imports must never be shadowed by a same-named vcpkg DLL.
  POST_EXCLUDE_REGEXES "^[A-Za-z]:[/\\\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\\\]"
  RESOLVED_DEPENDENCIES_VAR resolved
)

# Match by file name rather than resolved path: a DLL staged by an earlier
# build resolves next to EXE and must still be refreshed from DLL_DIR.
get_filename_component(exe_dir "${EXE}" DIRECTORY)
foreach(dependency IN LISTS resolved)
  get_filename_component(dll_name "${dependency}" NAME)
  if(EXISTS "${DLL_DIR}/${dll_name}")
    file(COPY_FILE "${DLL_DIR}/${dll_name}" "${exe_dir}/${dll_name}" ONLY_IF_DIFFERENT)
  endif()
endforeach()
