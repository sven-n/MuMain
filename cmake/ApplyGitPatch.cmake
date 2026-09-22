cmake_minimum_required(VERSION 3.25)

# Applies PATCH_FILE to SOURCE_DIR with `git apply`, or accepts it as already
# applied. Used as a FetchContent PATCH_COMMAND, which re-runs on reconfigure.

if(NOT DEFINED SOURCE_DIR OR NOT IS_DIRECTORY "${SOURCE_DIR}")
    message(FATAL_ERROR "SOURCE_DIR must name an existing directory")
endif()
if(NOT DEFINED PATCH_FILE OR NOT EXISTS "${PATCH_FILE}")
    message(FATAL_ERROR "PATCH_FILE must name an existing file")
endif()

get_filename_component(SOURCE_DIR "${SOURCE_DIR}" ABSOLUTE)
get_filename_component(PATCH_FILE "${PATCH_FILE}" ABSOLUTE)

find_package(Git REQUIRED)

# A dependency unpacked from a tarball has no repository of its own. When the
# build directory lives inside this repository, git would otherwise discover the
# outer repository, resolve the patch paths against its root and silently skip
# them. Stopping repository discovery at the parent directory makes git apply the
# patch relative to SOURCE_DIR, while a dependency that is its own clone is
# still detected.
get_filename_component(source_parent "${SOURCE_DIR}" DIRECTORY)

function(run_git_apply out_result out_output)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env "GIT_CEILING_DIRECTORIES=${source_parent}"
                "${GIT_EXECUTABLE}" apply --unidiff-zero ${ARGN} "${PATCH_FILE}"
        WORKING_DIRECTORY "${SOURCE_DIR}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
    set(${out_result} "${result}" PARENT_SCOPE)
    set(${out_output} "${output}${error}" PARENT_SCOPE)
endfunction()

run_git_apply(forward_result forward_output --check)
if(forward_result EQUAL 0)
    run_git_apply(apply_result apply_output)
    if(NOT apply_result EQUAL 0)
        message(FATAL_ERROR "git apply failed:\n${apply_output}")
    endif()
    return()
endif()

run_git_apply(reverse_result reverse_output --reverse --check)
if(reverse_result EQUAL 0)
    return()
endif()

message(FATAL_ERROR
    "patch cannot be applied or recognized as already applied\n"
    "forward check:\n${forward_output}\n"
    "reverse check:\n${reverse_output}")
