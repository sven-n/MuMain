cmake_minimum_required(VERSION 3.25)

# The rendering parity counters: logged by the scene manager, and shown on the $glstats overlay
# (its markup names each value, its C++ reads each from the renderer and the frame profiler).
function(require_texts file_path)
    file(READ "${file_path}" source)
    foreach(required_text IN LISTS ARGN)
        string(FIND "${source}" "${required_text}" position)
        if(position EQUAL -1)
            message(FATAL_ERROR "missing rendering parity diagnostic in ${file_path}: ${required_text}")
        endif()
    endforeach()
endfunction()

require_texts("${MU_SCENE_MANAGER_SOURCE}"
    "[RENDER diag] requested={} submitted={} pipeline_binds={} sampler_binds={}"
    "glyph_uploads={} skin_gpu={} skin_cpu_ineligible={} skin_failed={}"
)
require_texts("${MU_DIAGNOSTICS_OVERLAY_SOURCE}"
    "stats.pipelineBinds"
    "stats.samplerBinds"
    "stats.vertexUniformPushes"
    "stats.fragmentUniformPushes"
    "stats.merged2DDrawCalls"
    "Counter::GlyphUploads"
    "Counter::GpuSkinningSubmissions"
    "Counter::CpuSkinningIneligible"
    "Counter::GpuSkinningFailures"
    "Counter::BatchDraws"
)
require_texts("${MU_DIAGNOSTICS_DOCUMENT}"
    "{{pipeline_binds}}"
    "{{sampler_binds}}"
    "{{vertex_uniforms}}"
    "{{fragment_uniforms}}"
    "{{merged_2d}}"
    "{{glyphs_uploaded}}"
    "{{skin_gpu}}"
    "{{skin_cpu_ineligible}}"
    "{{skin_failed}}"
    "{{batch_draws}}"
)

message(STATUS "Rendering parity diagnostics: OK")
