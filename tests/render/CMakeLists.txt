mu_add_test(
    NAME frame_pixel_readback_tests
    SOURCES
        test_frame_pixel_readback.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/DiagnosticFrameCaptureSchedule.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/DiagnosticFrameCaptureWriter.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/FramePixelReadback.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/SdlGpuPixelFormat.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Scenes/ScreenshotCaptureState.cpp
    LINK_LIBS
        SDL3::SDL3
)

mu_add_test(
    NAME quad_topology_tests
    SOURCES
        test_quad_topology.cpp
)

mu_add_test(
    NAME aura_joint_lifecycle_tests
    SOURCES
        test_aura_joint_lifecycle.cpp
)

add_test(
    NAME imgui_sdlgpu_backend_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SOURCE_DIR=${CMAKE_SOURCE_DIR}/src"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_imgui_sdlgpu_backend.cmake"
)

add_test(
    NAME renderer_pipeline_fail_fast
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_RENDERER_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_pipeline_fail_fast.cmake"
)

add_test(
    NAME world_clear_color_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SCENE_MANAGER_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Scenes/SceneManager.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_world_clear_color.cmake"
)

add_test(
    NAME checked_in_shader_blob_validation
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SHADER_BLOB_DIR=${MU_SHADER_COMPILED_DIR}"
        -P "${MU_SHADER_VALIDATION_SCRIPT}"
)

add_test(
    NAME staged_shader_blob_validation
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SHADER_BLOB_DIR=${MU_SHADER_OUTPUT_DIR}"
        -P "${MU_SHADER_VALIDATION_SCRIPT}"
)

if(GLSLANG_EXE)
    set(MU_TEST_GLSLANG_EXE "${GLSLANG_EXE}")
else()
    unset(MU_TEST_GLSLANG_EXE CACHE)
    unset(MU_TEST_GLSLANG_EXE)
    find_program(MU_TEST_GLSLANG_EXE NAMES glslangValidator glslang)
endif()
if(SPIRV_CROSS_EXE)
    set(MU_TEST_SPIRV_CROSS_EXE "${SPIRV_CROSS_EXE}")
else()
    unset(MU_TEST_SPIRV_CROSS_EXE CACHE)
    unset(MU_TEST_SPIRV_CROSS_EXE)
    find_program(MU_TEST_SPIRV_CROSS_EXE NAMES spirv-cross)
endif()

if(MU_TEST_SPIRV_CROSS_EXE)
    add_test(
        NAME checked_in_shader_reflection_validation
        COMMAND "${CMAKE_COMMAND}"
            "-DMU_SHADER_BLOB_DIR=${MU_SHADER_COMPILED_DIR}"
            "-DMU_RENDERER_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp"
            "-DSPIRV_CROSS_EXE=${MU_TEST_SPIRV_CROSS_EXE}"
            -P "${CMAKE_SOURCE_DIR}/cmake/ValidateShaderReflection.cmake"
    )
    add_test(
        NAME staged_shader_reflection_validation
        COMMAND "${CMAKE_COMMAND}"
            "-DMU_SHADER_BLOB_DIR=${MU_SHADER_OUTPUT_DIR}"
            "-DMU_RENDERER_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp"
            "-DSPIRV_CROSS_EXE=${MU_TEST_SPIRV_CROSS_EXE}"
            -P "${CMAKE_SOURCE_DIR}/cmake/ValidateShaderReflection.cmake"
    )
endif()

if(MU_TEST_GLSLANG_EXE AND MU_TEST_SPIRV_CROSS_EXE)
    add_test(
        NAME shader_reflection_validation
        COMMAND "${CMAKE_COMMAND}"
            "-DMU_SHADER_SOURCE_DIR=${MU_SHADER_SOURCE_DIR}"
            "-DMU_SHADER_REFLECTION_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/shader-reflection"
            "-DMU_RENDERER_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp"
            "-DGLSLANG_EXE=${MU_TEST_GLSLANG_EXE}"
            "-DSPIRV_CROSS_EXE=${MU_TEST_SPIRV_CROSS_EXE}"
            -P "${CMAKE_SOURCE_DIR}/cmake/ValidateShaderReflection.cmake"
    )
endif()

if(MU_ENABLE_SHADER_COMPILATION
        AND DXC_EXE
        AND EXISTS "${DXC_EXE}"
        AND NOT IS_DIRECTORY "${DXC_EXE}")
    add_test(
        NAME checked_in_dxil_reflection_validation
        COMMAND "${CMAKE_COMMAND}"
            "-DMU_SHADER_BLOB_DIR=${MU_SHADER_COMPILED_DIR}"
            "-DDXC_EXE=${DXC_EXE}"
            -P "${CMAKE_SOURCE_DIR}/cmake/ValidateDxilReflection.cmake"
    )
    add_test(
        NAME staged_dxil_reflection_validation
        COMMAND "${CMAKE_COMMAND}"
            "-DMU_SHADER_BLOB_DIR=${MU_SHADER_OUTPUT_DIR}"
            "-DDXC_EXE=${DXC_EXE}"
            -P "${CMAKE_SOURCE_DIR}/cmake/ValidateDxilReflection.cmake"
    )
endif()
