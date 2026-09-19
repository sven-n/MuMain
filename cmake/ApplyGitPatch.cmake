cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_DIR OR NOT IS_DIRECTORY "${SOURCE_DIR}")
    message(FATAL_ERROR "SOURCE_DIR must name an existing directory")
endif()
if(NOT DEFINED PATCH_FILE OR NOT EXISTS "${PATCH_FILE}")
    message(FATAL_ERROR "PATCH_FILE must name an existing file")
endif()

get_filename_component(SOURCE_DIR "${SOURCE_DIR}" ABSOLUTE)
get_filename_component(PATCH_FILE "${PATCH_FILE}" ABSOLUTE)

find_package(Git REQUIRED)

execute_process(
    COMMAND "${GIT_EXECUTABLE}" apply --unidiff-zero --check "${PATCH_FILE}"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE forward_result
    OUTPUT_VARIABLE forward_output
    ERROR_VARIABLE forward_error
)
if(forward_result EQUAL 0)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" apply --unidiff-zero "${PATCH_FILE}"
        WORKING_DIRECTORY "${SOURCE_DIR}"
        RESULT_VARIABLE apply_result
        OUTPUT_VARIABLE apply_output
        ERROR_VARIABLE apply_error
    )
    if(NOT apply_result EQUAL 0)
        message(FATAL_ERROR "git apply failed:\n${apply_output}${apply_error}")
    endif()
    return()
endif()

execute_process(
    COMMAND "${GIT_EXECUTABLE}" apply --unidiff-zero --reverse --check "${PATCH_FILE}"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE reverse_result
    OUTPUT_VARIABLE reverse_output
    ERROR_VARIABLE reverse_error
)
if(reverse_result EQUAL 0)
    return()
endif()

message(FATAL_ERROR
    "patch cannot be applied or recognized as already applied\n"
    "forward check:\n${forward_output}${forward_error}\n"
    "reverse check:\n${reverse_output}${reverse_error}")
