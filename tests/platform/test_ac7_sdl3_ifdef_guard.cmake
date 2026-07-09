# Story 2.1.1: AC-7 — All SDL3 .cpp/.h files have #ifdef MU_ENABLE_SDL3 guard
# RED PHASE: SDL3 source files do not exist yet.
#
# Validates that every .cpp and .h file in Platform/sdl3/ directory
# contains an #ifdef MU_ENABLE_SDL3 or #if defined(MU_ENABLE_SDL3) guard.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED PLATFORM_DIR)
    message(FATAL_ERROR "PLATFORM_DIR must be set to the Platform source directory")
endif()

set(SDL3_DIR "${PLATFORM_DIR}/sdl3")

# Check that sdl3/ directory exists
if(NOT IS_DIRECTORY "${SDL3_DIR}")
    message(FATAL_ERROR "AC-7 FAIL: Platform/sdl3/ directory does not exist")
endif()

# Find all .cpp and .h files in sdl3/
file(GLOB sdl3_files "${SDL3_DIR}/*.cpp" "${SDL3_DIR}/*.h")

if(NOT sdl3_files)
    message(FATAL_ERROR "AC-7 FAIL: No .cpp or .h files found in Platform/sdl3/")
endif()

foreach(src_file ${sdl3_files})
    file(READ "${src_file}" file_content)
    get_filename_component(file_name "${src_file}" NAME)

    # Check for #ifdef MU_ENABLE_SDL3 or #if defined(MU_ENABLE_SDL3)
    string(FIND "${file_content}" "MU_ENABLE_SDL3" guard_pos)
    if(guard_pos EQUAL -1)
        message(FATAL_ERROR "AC-7 FAIL: ${file_name} does not contain MU_ENABLE_SDL3 guard")
    endif()
endforeach()

list(LENGTH sdl3_files file_count)
message(STATUS "AC-7 PASS: All ${file_count} SDL3 source files contain MU_ENABLE_SDL3 guard")
