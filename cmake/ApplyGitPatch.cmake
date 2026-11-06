cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_DIR OR NOT IS_DIRECTORY "${SOURCE_DIR}")
    message(FATAL_ERROR "SOURCE_DIR must name an existing directory")
endif()
if(NOT DEFINED PATCH_FILE OR NOT EXISTS "${PATCH_FILE}")
    message(FATAL_ERROR "PATCH_FILE must name an existing file")
endif()
# A patch step passes the SHA-256 the configure read, so its command changes with the patch. Another patch here means
# a wrong value or a patch edited after the configure.
if(DEFINED PATCH_SHA256)
    file(SHA256 "${PATCH_FILE}" patch_sha256)
    if(NOT patch_sha256 STREQUAL PATCH_SHA256)
        message(FATAL_ERROR
            "${PATCH_FILE} has the SHA-256 ${patch_sha256}, not ${PATCH_SHA256} as the configure read; configure again")
    endif()
endif()

# git searches upward from the real path, so the ceiling below must be computed from it too.
file(REAL_PATH "${SOURCE_DIR}" SOURCE_DIR)
get_filename_component(PATCH_FILE "${PATCH_FILE}" ABSOLUTE)

# A source tree without its own .git (a downloaded archive) sits inside this project's checkout.
# git apply would treat it as a subdirectory of that repository, skip every file and still
# succeed. Stop git from looking above SOURCE_DIR so the patch applies to the tree itself.
get_filename_component(source_parent "${SOURCE_DIR}" DIRECTORY)
set(ENV{GIT_CEILING_DIRECTORIES} "${source_parent}")
unset(ENV{GIT_DIR})
unset(ENV{GIT_WORK_TREE})
# The skip check below reads git's messages, which are translated in other locales.
set(ENV{LC_ALL} C)
set(ENV{LANGUAGE} C)

find_package(Git REQUIRED)

# Runs git apply verbosely in SOURCE_DIR. git only reports a skipped file in its verbose output,
# so a skip (git still found an outer repository) is an error instead of a silent success.
# Callers that can should still check the patched files themselves.
function(run_git_apply result_variable output_variable)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" apply --verbose --unidiff-zero ${ARGN} "${PATCH_FILE}"
        WORKING_DIRECTORY "${SOURCE_DIR}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE output
    )
    if(output MATCHES "Skipped patch")
        message(FATAL_ERROR "git apply skipped files of ${PATCH_FILE} in ${SOURCE_DIR}:\n${output}")
    endif()
    set(${result_variable} "${result}" PARENT_SCOPE)
    set(${output_variable} "${output}" PARENT_SCOPE)
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
