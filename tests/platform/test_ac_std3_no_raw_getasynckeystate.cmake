# Negative policy scan: compilation cannot reject new legacy input calls because
# the compatibility shim deliberately keeps existing calls buildable.
cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be set")
endif()

file(GLOB_RECURSE source_files
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
)

set(unexpected_files "")
foreach(source_file IN LISTS source_files)
    file(READ "${source_file}" source)
    string(REGEX REPLACE "//[^\n\r]*" "" source "${source}")
    string(REGEX REPLACE "/\\*([^*]|\\*+[^*/])*\\*+/" "" source "${source}")
    string(REGEX MATCH "(^|[^A-Za-z0-9_])GetAsyncKeyState[ \t\r\n]*\\(" match "${source}")
    if(NOT match)
        continue()
    endif()

    file(RELATIVE_PATH relative_file "${SOURCE_DIR}" "${source_file}")
    if(relative_file MATCHES "^((App|Core)/Platform|ThirdParty)/")
        continue()
    endif()

    list(APPEND unexpected_files "${relative_file}")
endforeach()

if(unexpected_files)
    list(JOIN unexpected_files "\n  " unexpected_list)
    message(FATAL_ERROR
        "New GetAsyncKeyState calls must use the SDL input abstraction:\n  ${unexpected_list}")
endif()
