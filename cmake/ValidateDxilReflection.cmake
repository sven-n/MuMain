cmake_minimum_required(VERSION 3.25)

foreach(required_variable IN ITEMS MU_SHADER_BLOB_DIR DXC_EXE)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

function(mu_add_dxil_validation_error error_message)
    string(REPLACE ";" "," sanitized_error "${error_message}")
    set_property(GLOBAL APPEND PROPERTY MU_DXIL_VALIDATION_ERRORS "${sanitized_error}")
endfunction()

set_property(GLOBAL PROPERTY MU_DXIL_VALIDATION_ERRORS "")

# shader|expected resource bindings; '-' means no resources.
set(shader_specs
    "basic_textured.vert|cb0,space1"
    "basic_textured.frag|cb0,space3+s0,space2+t0,space2"
    "basic_colored.vert|cb0,space1"
    "basic_colored.frag|-"
    "shadow_volume.vert|cb0,space1"
    "skinned_textured.vert|cb0,space1+t0,space0"
)

foreach(shader_spec IN LISTS shader_specs)
    string(REPLACE "|" ";" shader_fields "${shader_spec}")
    list(GET shader_fields 0 shader_name)
    list(GET shader_fields 1 expected_binding_text)
    set(dxil_path "${MU_SHADER_BLOB_DIR}/${shader_name}.dxil")

    execute_process(
        COMMAND "${DXC_EXE}" -dumpbin "${dxil_path}"
        RESULT_VARIABLE dump_result
        OUTPUT_VARIABLE dump_output
        ERROR_VARIABLE dump_error
    )
    if(NOT dump_result EQUAL 0)
        mu_add_dxil_validation_error(
            "${shader_name}: dxc -dumpbin failed (${dump_result}): ${dump_output}${dump_error}")
        continue()
    endif()

    string(FIND "${dump_output}" "Resource Bindings:" binding_table_start)
    string(FIND "${dump_output}" "ViewId state:" binding_table_end)
    if(binding_table_start EQUAL -1 OR binding_table_end EQUAL -1)
        mu_add_dxil_validation_error("${shader_name}: resource binding table not found")
        continue()
    endif()
    math(EXPR binding_table_length "${binding_table_end} - ${binding_table_start}")
    string(SUBSTRING "${dump_output}" ${binding_table_start} ${binding_table_length} binding_table)
    string(REGEX MATCHALL "(cb|[stu])[0-9]+(,space[0-9]+)?" actual_bindings "${binding_table}")
    set(normalized_bindings)
    foreach(actual_binding IN LISTS actual_bindings)
        if(NOT actual_binding MATCHES ",space")
            string(APPEND actual_binding ",space0")
        endif()
        list(APPEND normalized_bindings "${actual_binding}")
    endforeach()
    set(actual_bindings "${normalized_bindings}")
    list(SORT actual_bindings)
    if(expected_binding_text STREQUAL "-")
        set(expected_bindings)
    else()
        string(REPLACE "+" ";" expected_bindings "${expected_binding_text}")
        list(SORT expected_bindings)
    endif()

    if(NOT "${actual_bindings}" STREQUAL "${expected_bindings}")
        list(JOIN actual_bindings ", " actual_binding_text)
        list(JOIN expected_bindings ", " expected_binding_text)
        mu_add_dxil_validation_error(
            "${shader_name}: bindings [${actual_binding_text}], expected [${expected_binding_text}]")
    endif()
endforeach()

get_property(validation_errors GLOBAL PROPERTY MU_DXIL_VALIDATION_ERRORS)
if(validation_errors)
    list(JOIN validation_errors "\n  - " validation_details)
    message(FATAL_ERROR "DXIL reflection validation failed:\n  - ${validation_details}")
endif()

message(STATUS "Validated DXIL resource bindings for 6 shaders")
