cmake_minimum_required(VERSION 3.25)

# The terrain layers draw after the whole ground pass, so they must stay depth-tested: a layer passes
# over its own ground (same vertices, less or equal) and nearer ground drawn earlier hides it.
if(NOT DEFINED MU_TERRAIN_SOURCE)
    message(FATAL_ERROR "MU_TERRAIN_SOURCE must be set")
endif()

file(READ "${MU_TERRAIN_SOURCE}" terrain_source)

# The text from `start` up to `end`, which must both exist in that order.
function(isolate out_var start end)
    string(FIND "${terrain_source}" "${start}" start_position)
    string(FIND "${terrain_source}" "${end}" end_position)
    if(start_position EQUAL -1 OR end_position LESS start_position)
        message(FATAL_ERROR "Could not isolate ${start}")
    endif()
    math(EXPR length "${end_position} - ${start_position}")
    string(SUBSTRING "${terrain_source}" ${start_position} ${length} text)
    set(${out_var} "${text}" PARENT_SCOPE)
endfunction()

isolate(layer_draw "static void RenderLayerFaces()" "void RenderFaceAlpha(")
isolate(layer_faces "void RenderFaceAlpha(" "void FaceTexture(")
isolate(terrain_pass "void RenderTerrain(bool EditFlag)" "void RenderTerrain_After(")

foreach(forbidden IN ITEMS "DisableDepthTest()" "SetDepthTest(false)")
    foreach(part IN ITEMS layer_draw layer_faces)
        string(FIND "${${part}}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "Terrain layers must stay depth-tested: ${part} calls ${forbidden}")
        endif()
    endforeach()
endforeach()

string(FIND "${layer_draw}" "EnableDepthTest();" layer_enable)
if(layer_enable EQUAL -1)
    message(FATAL_ERROR "RenderLayerFaces() must turn the depth test on before drawing the layers")
endif()

string(FIND "${terrain_pass}" "EnableDepthTest();" pass_enable)
string(FIND "${terrain_pass}" "RenderTerrainFrustrum(EditFlag);" ground_pass)
string(FIND "${terrain_pass}" "RenderLayerFaces();" layer_pass)
if(pass_enable EQUAL -1 OR ground_pass EQUAL -1 OR pass_enable GREATER ground_pass)
    message(FATAL_ERROR "RenderTerrain() must turn the depth test on before the ground pass")
endif()
if(layer_pass EQUAL -1 OR layer_pass LESS ground_pass)
    message(FATAL_ERROR "RenderTerrain() must draw the layers after the ground pass")
endif()

message(STATUS "Validated terrain layer depth state")
