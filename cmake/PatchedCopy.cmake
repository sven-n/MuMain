# mu_patched_copy(SOURCE <file> PATCH <patch> OUTPUT_DIR <dir> MARKER <text>)
#
# Copies SOURCE into OUTPUT_DIR and applies PATCH to the copy. SOURCE itself is never changed, so a
# cached or overridden dependency and a revised patch cannot conflict. git's exit code alone does
# not prove that the patch landed, so the copy must contain MARKER. The inputs and the copy are
# configure dependencies, so a build reconfigures when an input changed or the copy was removed or
# edited, and the copy is then made again; otherwise configure does no work.
function(mu_patched_copy)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE;PATCH;OUTPUT_DIR;MARKER" "")
    set(apply_script "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ApplyGitPatch.cmake")
    get_filename_component(name "${arg_SOURCE}" NAME)
    set(output "${arg_OUTPUT_DIR}/${name}")
    set(stamp_file "${output}.stamp")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${arg_SOURCE}" "${arg_PATCH}" "${apply_script}" "${output}")

    file(SHA256 "${arg_SOURCE}" source_hash)
    file(SHA256 "${arg_PATCH}" patch_hash)
    file(SHA256 "${apply_script}" script_hash)
    set(inputs "${source_hash} ${patch_hash} ${script_hash}")
    if(EXISTS "${output}" AND EXISTS "${stamp_file}")
        file(SHA256 "${output}" output_hash)
        file(READ "${stamp_file}" stamp)
        if(stamp STREQUAL "${inputs} ${output_hash}")
            return()
        endif()
    endif()

    set(staging "${CMAKE_BINARY_DIR}/patched-copy-staging/${name}")
    file(REMOVE_RECURSE "${staging}")
    file(MAKE_DIRECTORY "${staging}" "${arg_OUTPUT_DIR}")
    file(COPY_FILE "${arg_SOURCE}" "${staging}/${name}")
    # The script finds git on its own; pass the configured one so a git outside PATH works too.
    set(git_argument "")
    if(GIT_EXECUTABLE)
        set(git_argument "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}")
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}" ${git_argument} "-DSOURCE_DIR=${staging}" "-DPATCH_FILE=${arg_PATCH}"
            -P "${apply_script}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE log
        ERROR_VARIABLE log
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${arg_PATCH} does not apply to ${arg_SOURCE}:\n${log}")
    endif()
    file(STRINGS "${staging}/${name}" marker_lines REGEX "${arg_MARKER}" LIMIT_COUNT 1)
    if(NOT marker_lines)
        message(FATAL_ERROR "git applied ${arg_PATCH}, but the copy of ${name} lacks \"${arg_MARKER}\":\n${log}")
    endif()

    # Only a changed copy is written, so dependent code is not rebuilt for nothing.
    file(COPY_FILE "${staging}/${name}" "${output}" ONLY_IF_DIFFERENT)
    file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/patched-copy-staging")
    file(SHA256 "${output}" output_hash)
    file(WRITE "${stamp_file}" "${inputs} ${output_hash}")
endfunction()
