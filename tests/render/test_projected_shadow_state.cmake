cmake_minimum_required(VERSION 3.25)

foreach(required_variable IN ITEMS MU_BMD_SOURCE MU_SHADOW_VOLUME_SOURCE)
    if(NOT DEFINED ${required_variable})
        message(FATAL_ERROR "${required_variable} must be set")
    endif()
endforeach()

file(READ "${MU_BMD_SOURCE}" bmd_source)
file(READ "${MU_SHADOW_VOLUME_SOURCE}" shadow_volume_source)

string(FIND "${bmd_source}" "void BMD::RenderBodyShadow(" shadow_start)
string(FIND "${bmd_source}" "void BMD::RenderObjectBoundingBox()" shadow_end)
if(shadow_start EQUAL -1 OR shadow_end LESS shadow_start)
    message(FATAL_ERROR "Could not isolate BMD::RenderBodyShadow")
endif()

math(EXPR shadow_length "${shadow_end} - ${shadow_start}")
string(SUBSTRING "${bmd_source}" ${shadow_start} ${shadow_length} projected_shadow_source)

foreach(required_call IN ITEMS AddMeshShadowTriangles AddClothesShadowTriangles)
    string(FIND "${projected_shadow_source}" "${required_call}(" call_position)
    if(call_position EQUAL -1)
        message(FATAL_ERROR "Projected body shadows must call ${required_call}")
    endif()
endforeach()

foreach(forbidden_state IN ITEMS SetStencilTest SetStencilOp GL_STENCIL_TEST glStencil)
    string(FIND "${projected_shadow_source}" "${forbidden_state}" state_position)
    if(NOT state_position EQUAL -1)
        message(FATAL_ERROR
            "Projected body shadows must not enable unsupported stencil state: ${forbidden_state}")
    endif()
endforeach()

string(FIND "${shadow_volume_source}" "SetStencilTest(true)" volume_stencil_position)
if(volume_stencil_position EQUAL -1)
    message(FATAL_ERROR "Full shadow-volume rendering must remain explicitly stencil-dependent")
endif()

message(STATUS "Validated projected-shadow and shadow-volume stencil boundaries")
