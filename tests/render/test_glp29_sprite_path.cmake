file(READ "${RENDER_UTIL_SOURCE}" render_util)

string(FIND "${render_util}" "void RenderSprite(" render_sprite_begin)
string(FIND "${render_util}" "void RenderSpriteUV(" render_sprite_end)
if(render_sprite_begin EQUAL -1 OR render_sprite_end EQUAL -1 OR render_sprite_end LESS render_sprite_begin)
    message(FATAL_ERROR "Unable to isolate RenderSprite")
endif()

math(EXPR render_sprite_length "${render_sprite_end} - ${render_sprite_begin}")
string(SUBSTRING "${render_util}" ${render_sprite_begin} ${render_sprite_length} render_sprite)

foreach(contract
        "const float rad = Rotation * (Q_PI * 2 / 360);"
        "p[i][0] = x + dx[i] * cy - dy[i] * sy;"
        "p[i][1] = y + dx[i] * sy + dy[i] * cy;"
        "const mu::Vertex3D vertices[6]")
    string(FIND "${render_sprite}" "${contract}" contract_pos)
    if(contract_pos EQUAL -1)
        message(FATAL_ERROR "Missing GLP-29 sprite contract: ${contract}")
    endif()
endforeach()

string(FIND "${render_sprite}" "AngleMatrix(Angle, Matrix);" old_rotation_pos)
if(NOT old_rotation_pos EQUAL -1)
    message(FATAL_ERROR "RenderSprite still builds a 3D angle matrix for Z-only rotation")
endif()

file(READ "${RENDERER_SOURCE}" renderer)
string(FIND "${renderer}" "std::vector<IRVertex>" heap_quad_scratch_pos)
if(NOT heap_quad_scratch_pos EQUAL -1)
    message(FATAL_ERROR "SDL GPU renderer restored the retired heap-backed IR quad scratch")
endif()
