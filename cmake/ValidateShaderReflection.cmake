cmake_minimum_required(VERSION 3.25)

if(DEFINED MU_SHADER_BLOB_DIR AND NOT MU_SHADER_BLOB_DIR STREQUAL "")
    if(NOT IS_DIRECTORY "${MU_SHADER_BLOB_DIR}")
        message(FATAL_ERROR "Shader blob directory does not exist: ${MU_SHADER_BLOB_DIR}")
    endif()
    set(required_variables MU_RENDERER_SOURCE SPIRV_CROSS_EXE)
    set(blob_mode TRUE)
else()
    set(required_variables
        MU_SHADER_SOURCE_DIR
        MU_SHADER_REFLECTION_OUTPUT_DIR
        MU_RENDERER_SOURCE
        GLSLANG_EXE
        SPIRV_CROSS_EXE)
    set(blob_mode FALSE)
endif()

foreach(required_variable IN LISTS required_variables)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

function(mu_add_shader_validation_error error_message)
    string(REPLACE ";" "," sanitized_error "${error_message}")
    set_property(GLOBAL APPEND PROPERTY MU_SHADER_VALIDATION_ERRORS "${sanitized_error}")
endfunction()

function(mu_json_array_length output_variable json member)
    string(JSON array_length ERROR_VARIABLE json_error LENGTH "${json}" "${member}")
    if(json_error STREQUAL "NOTFOUND")
        set(${output_variable} "${array_length}" PARENT_SCOPE)
    else()
        set(${output_variable} 0 PARENT_SCOPE)
    endif()
endfunction()

function(mu_expect_resource_bindings shader_name json member expected_count expected_set)
    mu_json_array_length(actual_count "${json}" "${member}")
    if(NOT actual_count EQUAL expected_count)
        mu_add_shader_validation_error(
            "${shader_name}: ${member} count ${actual_count}, expected ${expected_count}")
        return()
    endif()

    if(expected_count EQUAL 0)
        return()
    endif()

    math(EXPR last_resource "${expected_count} - 1")
    foreach(resource_index RANGE 0 ${last_resource})
        string(JSON actual_set GET "${json}" "${member}" ${resource_index} set)
        string(JSON actual_binding GET "${json}" "${member}" ${resource_index} binding)
        if(NOT actual_set EQUAL expected_set OR NOT actual_binding EQUAL resource_index)
            mu_add_shader_validation_error(
                "${shader_name}: ${member}[${resource_index}] set/binding ${actual_set}/${actual_binding}, expected ${expected_set}/${resource_index}")
        endif()
    endforeach()
endfunction()

function(mu_expect_input_locations shader_name json expected_locations)
    mu_json_array_length(input_count "${json}" inputs)
    if(expected_locations STREQUAL "")
        set(expected_count 0)
    else()
        string(REPLACE "," ";" expected_list "${expected_locations}")
        list(LENGTH expected_list expected_count)
    endif()

    if(NOT input_count EQUAL expected_count)
        mu_add_shader_validation_error(
            "${shader_name}: vertex input count ${input_count}, expected ${expected_count}")
        return()
    endif()

    if(expected_count EQUAL 0)
        return()
    endif()

    set(actual_locations)
    math(EXPR last_input "${input_count} - 1")
    foreach(input_index RANGE 0 ${last_input})
        string(JSON input_location GET "${json}" inputs ${input_index} location)
        list(APPEND actual_locations "${input_location}")
    endforeach()
    list(SORT actual_locations COMPARE NATURAL)

    if(NOT actual_locations STREQUAL expected_list)
        list(JOIN actual_locations "," actual_location_text)
        list(JOIN expected_list "," expected_location_text)
        mu_add_shader_validation_error(
            "${shader_name}: vertex input locations ${actual_location_text}, expected ${expected_location_text}")
    endif()
endfunction()

function(mu_expect_hlsl_semantics shader_name hlsl expected_locations)
    string(REGEX MATCH "struct[ \t\r\n]+VSInput[ \t\r\n]*\\{([^}]*)\\}" input_struct "${hlsl}")
    if(input_struct STREQUAL "")
        mu_add_shader_validation_error("${shader_name}: VSInput declaration not found")
        return()
    endif()

    string(REGEX MATCHALL "TEXCOORD[0-9]+" semantic_matches "${CMAKE_MATCH_1}")
    set(actual_locations)
    foreach(semantic IN LISTS semantic_matches)
        string(REGEX REPLACE "TEXCOORD" "" semantic_index "${semantic}")
        list(APPEND actual_locations "${semantic_index}")
    endforeach()
    string(REPLACE "," ";" expected_list "${expected_locations}")
    if(NOT actual_locations STREQUAL expected_list)
        list(JOIN actual_locations "," actual_location_text)
        list(JOIN expected_list "," expected_location_text)
        mu_add_shader_validation_error(
            "${shader_name}: HLSL input semantics ${actual_location_text}, expected TEXCOORD indices ${expected_location_text}")
    endif()
endfunction()

function(mu_expect_entry_point_stages shader_name json stage msl)
    mu_json_array_length(entry_point_count "${json}" entryPoints)
    if(entry_point_count EQUAL 1)
        string(JSON entry_point_name GET "${json}" entryPoints 0 name)
        string(JSON entry_point_mode GET "${json}" entryPoints 0 mode)
        if(NOT entry_point_name STREQUAL "main" OR NOT entry_point_mode STREQUAL stage)
            mu_add_shader_validation_error(
                "${shader_name}: SPIR-V entry point ${entry_point_name}/${entry_point_mode}, expected main/${stage}")
        endif()
    else()
        mu_add_shader_validation_error(
            "${shader_name}: SPIR-V entry point count ${entry_point_count}, expected 1")
    endif()

    if(stage STREQUAL "vert")
        set(expected_msl_stage vertex)
    else()
        set(expected_msl_stage fragment)
    endif()
    string(REGEX MATCH
        "(^|[\r\n])[ \t]*(vertex|fragment)[^\r\n]*[ \t]+main0[ \t]*\\("
        msl_entry_point "${msl}")
    if(msl_entry_point STREQUAL "")
        mu_add_shader_validation_error(
            "${shader_name}: MSL main0 entry point stage not found, expected ${expected_msl_stage}")
    elseif(NOT CMAKE_MATCH_2 STREQUAL expected_msl_stage)
        mu_add_shader_validation_error(
            "${shader_name}: MSL main0 entry point stage ${CMAKE_MATCH_2}, expected ${expected_msl_stage}")
    endif()
endfunction()

function(mu_expect_renderer_metadata shader_name expected_samplers expected_storage expected_uniforms)
    string(REPLACE "." ";" shader_parts "${shader_name}")
    list(GET shader_parts 0 source_name)
    list(GET shader_parts 1 stage)
    string(REGEX MATCH
        "createShader\\(\"${source_name}\",[ \t\r\n]*\"${stage}\",[ \t\r\n]*SDL_GPU_SHADERSTAGE_[A-Z]+,[ \t\r\n]*([0-9]+),[ \t\r\n]*([0-9]+),[ \t\r\n]*([0-9]+),"
        metadata_match "${renderer_source}")
    if(metadata_match STREQUAL "")
        mu_add_shader_validation_error("${shader_name}: LoadShaders() metadata not found")
        return()
    endif()

    if(NOT CMAKE_MATCH_1 EQUAL expected_samplers
            OR NOT CMAKE_MATCH_2 EQUAL expected_storage
            OR NOT CMAKE_MATCH_3 EQUAL expected_uniforms)
        mu_add_shader_validation_error(
            "${shader_name}: LoadShaders() metadata ${CMAKE_MATCH_1}/${CMAKE_MATCH_2}/${CMAKE_MATCH_3}, expected ${expected_samplers}/${expected_storage}/${expected_uniforms}")
    endif()
endfunction()

function(mu_expect_msl_indices shader_name msl resource_kind expected_count)
    string(REGEX MATCHALL "\\[\\[${resource_kind}\\(([0-9]+)\\)\\]\\]" resource_matches "${msl}")
    list(LENGTH resource_matches actual_count)
    if(NOT actual_count EQUAL expected_count)
        mu_add_shader_validation_error(
            "${shader_name}: MSL ${resource_kind} count ${actual_count}, expected ${expected_count}")
        return()
    endif()

    if(expected_count EQUAL 0)
        return()
    endif()

    math(EXPR last_resource "${expected_count} - 1")
    foreach(resource_index RANGE 0 ${last_resource})
        list(GET resource_matches ${resource_index} resource_match)
        if(NOT resource_match STREQUAL "[[${resource_kind}(${resource_index})]]")
            mu_add_shader_validation_error(
                "${shader_name}: non-contiguous MSL ${resource_kind} index at ${resource_match}")
        endif()
    endforeach()
endfunction()

function(mu_expect_msl_layout shader_name stage msl sampler_count storage_count uniform_count)
    if(stage STREQUAL "vert" AND NOT msl MATCHES "\\[\\[stage_in\\]\\]")
        mu_add_shader_validation_error("${shader_name}: MSL vertex input does not use [[stage_in]]")
    endif()

    mu_expect_msl_indices("${shader_name}" "${msl}" texture "${sampler_count}")
    mu_expect_msl_indices("${shader_name}" "${msl}" sampler "${sampler_count}")
    math(EXPR buffer_count "${storage_count} + ${uniform_count}")
    mu_expect_msl_indices("${shader_name}" "${msl}" buffer "${buffer_count}")

    if(storage_count GREATER 0 AND uniform_count GREATER 0)
        string(REGEX MATCHALL "((constant)|(device))[^,\n]*\\[\\[buffer\\([0-9]+\\)\\]\\]" buffer_parameters "${msl}")
        list(LENGTH buffer_parameters parameter_count)
        if(NOT parameter_count EQUAL buffer_count)
            mu_add_shader_validation_error(
                "${shader_name}: could not identify MSL uniform/storage buffer order")
            return()
        endif()
        list(GET buffer_parameters 0 first_buffer)
        list(GET buffer_parameters ${uniform_count} first_storage_buffer)
        if(NOT first_buffer MATCHES "constant" OR first_buffer MATCHES "device"
                OR NOT first_storage_buffer MATCHES "device")
            mu_add_shader_validation_error(
                "${shader_name}: MSL uniform buffers must precede storage buffers")
        endif()
    endif()
endfunction()

file(READ "${MU_RENDERER_SOURCE}" renderer_source)
if(NOT blob_mode)
    file(MAKE_DIRECTORY "${MU_SHADER_REFLECTION_OUTPUT_DIR}")
endif()
set_property(GLOBAL PROPERTY MU_SHADER_VALIDATION_ERRORS "")

# shader|stage|vertex locations|samplers|storage buffers|uniform buffers
set(shader_specs
    "basic_textured.vert|vert|0,1,2|0|0|1"
    "basic_textured.frag|frag||1|0|1"
    "basic_colored.vert|vert|0,1|0|0|1"
    "basic_colored.frag|frag||0|0|0"
    "shadow_volume.vert|vert|0|0|0|1"
    "skinned_textured.vert|vert|0,1,2,3,4,5|0|1|1"
)

foreach(shader_spec IN LISTS shader_specs)
    string(REPLACE "|" ";" shader_fields "${shader_spec}")
    list(GET shader_fields 0 shader_name)
    list(GET shader_fields 1 stage)
    list(GET shader_fields 2 input_locations)
    list(GET shader_fields 3 sampler_count)
    list(GET shader_fields 4 storage_count)
    list(GET shader_fields 5 uniform_count)

    if(blob_mode)
        set(spv_path "${MU_SHADER_BLOB_DIR}/${shader_name}.spv")
        set(msl_path "${MU_SHADER_BLOB_DIR}/${shader_name}.msl")
    else()
        set(source_path "${MU_SHADER_SOURCE_DIR}/${shader_name}.hlsl")
        set(spv_path "${MU_SHADER_REFLECTION_OUTPUT_DIR}/${shader_name}.spv")
        set(json_path "${MU_SHADER_REFLECTION_OUTPUT_DIR}/${shader_name}.json")
        set(msl_path "${MU_SHADER_REFLECTION_OUTPUT_DIR}/${shader_name}.msl")
        file(READ "${source_path}" hlsl)

        execute_process(
            COMMAND "${GLSLANG_EXE}" -V -D -S "${stage}" -e main -o "${spv_path}" "${source_path}"
            RESULT_VARIABLE compile_result
            OUTPUT_VARIABLE compile_output
            ERROR_VARIABLE compile_error
        )
        if(NOT compile_result EQUAL 0)
            mu_add_shader_validation_error("${shader_name}: glslang failed: ${compile_output}${compile_error}")
            continue()
        endif()
    endif()

    execute_process(
        COMMAND "${SPIRV_CROSS_EXE}" "${spv_path}" --reflect
        RESULT_VARIABLE reflect_result
        OUTPUT_VARIABLE reflection_json
        ERROR_VARIABLE reflect_error
    )
    if(NOT reflect_result EQUAL 0)
        mu_add_shader_validation_error("${shader_name}: reflection failed: ${reflect_error}")
        continue()
    endif()
    if(NOT blob_mode)
        file(WRITE "${json_path}" "${reflection_json}")

        execute_process(
            COMMAND "${SPIRV_CROSS_EXE}" "${spv_path}" --msl --output "${msl_path}"
            RESULT_VARIABLE msl_result
            ERROR_VARIABLE msl_error
        )
        if(NOT msl_result EQUAL 0)
            mu_add_shader_validation_error("${shader_name}: MSL generation failed: ${msl_error}")
            continue()
        endif()
    endif()
    file(READ "${msl_path}" msl)
    mu_expect_entry_point_stages("${shader_name}" "${reflection_json}" "${stage}" "${msl}")

    if(stage STREQUAL "vert")
        set(resource_set 0)
        set(uniform_set 1)
        mu_expect_input_locations("${shader_name}" "${reflection_json}" "${input_locations}")
        if(NOT blob_mode)
            mu_expect_hlsl_semantics("${shader_name}" "${hlsl}" "${input_locations}")
        endif()
    else()
        set(resource_set 2)
        set(uniform_set 3)
    endif()

    mu_expect_resource_bindings("${shader_name}" "${reflection_json}" separate_images "${sampler_count}" "${resource_set}")
    mu_expect_resource_bindings("${shader_name}" "${reflection_json}" separate_samplers "${sampler_count}" "${resource_set}")
    mu_expect_resource_bindings("${shader_name}" "${reflection_json}" ssbos "${storage_count}" "${resource_set}")
    mu_expect_resource_bindings("${shader_name}" "${reflection_json}" ubos "${uniform_count}" "${uniform_set}")
    mu_expect_resource_bindings("${shader_name}" "${reflection_json}" textures 0 "${resource_set}")
    mu_expect_resource_bindings("${shader_name}" "${reflection_json}" images 0 "${resource_set}")
    mu_expect_renderer_metadata("${shader_name}" "${sampler_count}" "${storage_count}" "${uniform_count}")
    mu_expect_msl_layout("${shader_name}" "${stage}" "${msl}" "${sampler_count}" "${storage_count}" "${uniform_count}")
endforeach()

get_property(validation_errors GLOBAL PROPERTY MU_SHADER_VALIDATION_ERRORS)
if(validation_errors)
    list(JOIN validation_errors "\n  - " validation_details)
    message(FATAL_ERROR "Shader reflection validation failed:\n  - ${validation_details}")
endif()

message(STATUS "Validated SDL_GPU reflection and MSL layout for 6 shaders")
