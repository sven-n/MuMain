file(READ "${SOURCE_DIR}/src/source/Core/Utilities/FrameProfiler.h" profiler)
file(READ "${SOURCE_DIR}/src/source/Core/Utilities/Log/muConsoleDebug.cpp" console)
file(READ "${SOURCE_DIR}/src/source/Render/Renderer/MuRenderer.h" renderer_api)
file(READ "${SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp" renderer)
file(READ "${SOURCE_DIR}/src/source/Scenes/SceneManager.cpp" scenes)

foreach(command "$glstats on" "$glstats off")
    string(FIND "${console}" "${command}" command_pos)
    if(command_pos EQUAL -1)
        message(FATAL_ERROR "Missing ${command} console command")
    endif()
endforeach()

foreach(contract "struct RendererStats" "SetStatsEnabled(bool" "RendererStats GetFrameStats() const")
    string(FIND "${renderer_api}" "${contract}" api_pos)
    if(api_pos EQUAL -1)
        message(FATAL_ERROR "Missing renderer stats API: ${contract}")
    endif()
endforeach()

foreach(contract "SetStatsEnabled(bool enabled) override" "GetFrameStats() const override")
    string(FIND "${renderer}" "${contract}" backend_pos)
    if(backend_pos EQUAL -1)
        message(FATAL_ERROR "Missing SDL GPU stats implementation: ${contract}")
    endif()
endforeach()

string(FIND "${scenes}" "GetFrameStats()" overlay_pos)
if(overlay_pos EQUAL -1)
    message(FATAL_ERROR "Stats overlay does not read renderer frame stats")
endif()

foreach(counter
        "BatchDraws"
        "BatchVertices"
        "BatchBreakTexture"
        "BatchBreakBlend"
        "BatchBreakDepth"
        "BatchBreakProgram"
        "BatchBreakUniform"
        "BatchBreakMatrix"
        "BatchBreakDraw"
        "BatchBreakOther")
    string(FIND "${profiler}" "${counter}" counter_pos)
    if(counter_pos EQUAL -1)
        message(FATAL_ERROR "Missing SDL GPU batch attribution counter: ${counter}")
    endif()
endforeach()

foreach(contract
        "ClassifyBatchBreak"
        "Counter::BatchDraws"
        "Counter::BatchVertices"
        "Counter::BatchBreakTexture"
        "Counter::BatchBreakBlend"
        "Counter::BatchBreakDepth"
        "Counter::BatchBreakProgram"
        "Counter::BatchBreakUniform"
        "Counter::BatchBreakMatrix"
        "Counter::BatchBreakDraw"
        "Counter::BatchBreakOther")
    string(FIND "${renderer}" "${contract}" attribution_pos)
    if(attribution_pos EQUAL -1)
        message(FATAL_ERROR "Missing SDL GPU batch-break attribution: ${contract}")
    endif()
endforeach()

foreach(label "Batch Draw:" "Break Tex:")
    string(FIND "${scenes}" "${label}" label_pos)
    if(label_pos EQUAL -1)
        message(FATAL_ERROR "Stats overlay is missing ${label}")
    endif()
endforeach()

string(FIND "${profiler}" "SDL_GL_GetProcAddress" raw_gl_pos)
if(NOT raw_gl_pos EQUAL -1)
    message(FATAL_ERROR "Generic FrameProfiler still owns raw OpenGL queries")
endif()
