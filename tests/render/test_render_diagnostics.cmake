cmake_minimum_required(VERSION 3.25)

file(READ "${MU_SCENE_MANAGER_SOURCE}" scene_source)

foreach(required_text IN ITEMS
    "Bind Pipe:%u Samp:%u VU:%u FU:%u"
    "2D Merge:%u Glyph upload:%u"
    "Skin GPU:%u CPU-ineligible:%u Failed:%u"
    "[RENDER diag] requested=%u submitted=%u pipeline_binds=%u sampler_binds=%u"
    "glyph_uploads=%u skin_gpu=%u skin_cpu_ineligible=%u skin_failed=%u"
)
    string(FIND "${scene_source}" "${required_text}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "missing rendering parity diagnostic: ${required_text}")
    endif()
endforeach()

message(STATUS "Rendering parity diagnostics: OK")
