mu_add_test(
    NAME bitmap_gpu_resources_tests
    SOURCES
        test_bitmap_gpu_resources.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/BitmapGpuResources.cpp
    LINK_LIBS SDL3::SDL3
)

# A standalone executable permits CTest's real skip return code when not opted in.
# The GPU regression still links the same production owner and SDL dependency as
# bitmap_gpu_resources_tests; unlike doctest discovery it runs as one timed case.
add_executable(bitmap_gpu_upload_integration test_bitmap_gpu_upload_integration.cpp
    ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/BitmapGpuResources.cpp)
target_include_directories(bitmap_gpu_upload_integration PRIVATE ${CMAKE_SOURCE_DIR}/src/source)
target_link_libraries(bitmap_gpu_upload_integration PRIVATE SDL3::SDL3)
add_test(NAME bitmap_gpu_upload_integration COMMAND bitmap_gpu_upload_integration)
set_tests_properties(bitmap_gpu_upload_integration PROPERTIES LABELS gpu SKIP_RETURN_CODE 77 TIMEOUT 180)

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
    NAME sdl_gpu_replay_state_tests
    SOURCES
        test_sdl_gpu_replay_state.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/SdlGpuReplayState.cpp
    LINK_LIBS SDL3::SDL3
)

mu_add_test(
    NAME sdl_gpu_validation_tests
    SOURCES
        test_sdl_gpu_validation.cpp
)

mu_add_test(
    NAME aura_joint_lifecycle_tests
    SOURCES
        test_aura_joint_lifecycle.cpp
)

mu_add_test(
    NAME gpu_skinning_path_tests
    SOURCES
        test_gpu_skinning_path.cpp
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
    NAME d3d12_culling_diagnostic_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_RENDERER_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_d3d12_culling_diagnostic.cmake"
)

add_test(
    NAME world_clear_color_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SCENE_MANAGER_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Scenes/SceneManager.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_world_clear_color.cmake"
)

add_test(
    NAME render_diagnostics_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SCENE_MANAGER_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Scenes/SceneManager.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_render_diagnostics.cmake"
)

add_test(
    NAME projected_shadow_state_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_BMD_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Models/ZzzBMD.cpp"
        "-DMU_SHADOW_VOLUME_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Models/ShadowVolume.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_projected_shadow_state.cmake"
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

add_executable(texture_failure_fixture test_texture_failure.cpp
    ${CMAKE_SOURCE_DIR}/src/source/Render/Textures/TextureFailure.cpp)
target_include_directories(texture_failure_fixture PRIVATE ${CMAKE_SOURCE_DIR}/src/source)
add_test(NAME texture_failure_emulator_arguments COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_texture_failure_emulator.py")

# Python owns the exit-status assertion, so CTest cannot automatically add the
# target's cross-compilation emulator to the nested executable invocation.
set(texture_failure_emulator)
if(CMAKE_CROSSCOMPILING)
    get_target_property(texture_failure_emulator texture_failure_fixture CROSSCOMPILING_EMULATOR)
    if(NOT texture_failure_emulator)
        set(texture_failure_emulator)
    endif()
endif()
add_test(NAME texture_failure_continue COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_texture_failure_subprocess.py" 0
    ${texture_failure_emulator} $<TARGET_FILE:texture_failure_fixture> continue
    COMMAND_EXPAND_LISTS)
add_test(NAME texture_failure_fatal COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_texture_failure_subprocess.py" 1
    ${texture_failure_emulator} $<TARGET_FILE:texture_failure_fixture> fatal
    COMMAND_EXPAND_LISTS)
add_test(NAME texture_failure_dispatch_continue COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_texture_failure_subprocess.py" 0
    ${texture_failure_emulator} $<TARGET_FILE:texture_failure_fixture> dispatch-continue
    COMMAND_EXPAND_LISTS)
add_test(NAME texture_failure_dispatch_fatal COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_texture_failure_subprocess.py" 1
    ${texture_failure_emulator} $<TARGET_FILE:texture_failure_fixture> dispatch-fatal
    COMMAND_EXPAND_LISTS)
add_test(NAME texture_failure_dispatch_decline COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_texture_failure_subprocess.py" 1
    ${texture_failure_emulator} $<TARGET_FILE:texture_failure_fixture> dispatch-decline
    COMMAND_EXPAND_LISTS)

mu_add_test(
    NAME bitmap_loader_tests
    SOURCES
        test_bitmap_loader.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Sprites/GlobalBitmap.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Sprites/GlobalBitmapLoad.cpp
    LINK_LIBS MuClient
)
