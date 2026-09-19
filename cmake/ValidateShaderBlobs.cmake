cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED MU_SHADER_BLOB_DIR OR MU_SHADER_BLOB_DIR STREQUAL "")
    message(FATAL_ERROR "MU_SHADER_BLOB_DIR is required")
endif()

if(NOT IS_DIRECTORY "${MU_SHADER_BLOB_DIR}")
    message(FATAL_ERROR "Shader blob directory does not exist: ${MU_SHADER_BLOB_DIR}")
endif()

set(MU_SHADER_STAGE_NAMES
    basic_textured.vert
    basic_textured.frag
    basic_colored.vert
    basic_colored.frag
    skinned_textured.vert
    shadow_volume.vert
)
set(MU_SHADER_BLOB_FORMATS spv msl dxil)
set(MU_SHADER_BLOB_NAMES)
block(SCOPE_FOR VARIABLES)
    foreach(shader_stage IN LISTS MU_SHADER_STAGE_NAMES)
        foreach(format IN LISTS MU_SHADER_BLOB_FORMATS)
            list(APPEND MU_SHADER_BLOB_NAMES "${shader_stage}.${format}")
        endforeach()
    endforeach()
    set(MU_SHADER_BLOB_NAMES "${MU_SHADER_BLOB_NAMES}" PARENT_SCOPE)
endblock()

block(SCOPE_FOR VARIABLES)
    set(shader_blob_errors)
    foreach(blob_name IN LISTS MU_SHADER_BLOB_NAMES)
        set(blob_path "${MU_SHADER_BLOB_DIR}/${blob_name}")
        if(IS_SYMLINK "${blob_path}")
            list(APPEND shader_blob_errors "${blob_name}: symbolic link is not allowed")
            continue()
        endif()
        if(IS_DIRECTORY "${blob_path}")
            list(APPEND shader_blob_errors "${blob_name}: directory is not allowed")
            continue()
        endif()
        if(NOT EXISTS "${blob_path}")
            list(APPEND shader_blob_errors "${blob_name}: missing file")
            continue()
        endif()

        file(SIZE "${blob_path}" blob_size)
        if(blob_size EQUAL 0)
            list(APPEND shader_blob_errors "${blob_name}: empty file")
            continue()
        endif()

        if(blob_name MATCHES "\\.spv$")
            file(READ "${blob_path}" blob_header LIMIT 4 HEX)
            if(NOT blob_header STREQUAL "03022307")
                list(APPEND shader_blob_errors "${blob_name}: invalid SPIR-V header ${blob_header}")
            endif()
        elseif(blob_name MATCHES "\\.dxil$")
            file(READ "${blob_path}" blob_header LIMIT 4 HEX)
            if(NOT blob_header STREQUAL "44584243")
                list(APPEND shader_blob_errors "${blob_name}: invalid DXIL header ${blob_header}")
            endif()
        else()
            file(READ "${blob_path}" blob_contents)
            string(FIND "${blob_contents}" " main0(" entry_point_position)
            if(entry_point_position EQUAL -1)
                list(APPEND shader_blob_errors "${blob_name}: missing MSL main0 entry point")
            endif()
        endif()
    endforeach()

    file(GLOB actual_shader_blobs LIST_DIRECTORIES FALSE
        "${MU_SHADER_BLOB_DIR}/*.spv"
        "${MU_SHADER_BLOB_DIR}/*.msl"
        "${MU_SHADER_BLOB_DIR}/*.dxil"
    )
    foreach(blob_path IN LISTS actual_shader_blobs)
        get_filename_component(blob_name "${blob_path}" NAME)
        if(NOT blob_name IN_LIST MU_SHADER_BLOB_NAMES)
            list(APPEND shader_blob_errors "${blob_name}: unexpected file")
        endif()
    endforeach()

    if(shader_blob_errors)
        list(JOIN shader_blob_errors "\n  - " shader_blob_error_text)
        message(FATAL_ERROR
            "Shader blob validation failed for ${MU_SHADER_BLOB_DIR}:\n"
            "  - ${shader_blob_error_text}"
        )
    endif()

    list(LENGTH MU_SHADER_BLOB_NAMES shader_blob_count)
    message(STATUS "Validated ${shader_blob_count} shader blobs in ${MU_SHADER_BLOB_DIR}")
endblock()
