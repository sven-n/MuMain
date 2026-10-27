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
    NAME reorderable_draw_runs_tests
    SOURCES
        test_reorderable_draw_runs.cpp
)

mu_add_test(
    NAME skinned_bone_palette_tests
    SOURCES
        test_skinned_bone_palette.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME overlay2d_recorder_tests
    SOURCES
        test_overlay2d_recorder.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/Overlay2DRecorder.cpp
)

mu_add_test(
    NAME quad_topology_tests
    SOURCES
        test_quad_topology.cpp
)

mu_add_test(
    NAME mip_chain_tests
    SOURCES
        test_mip_chain.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Textures/MipChain.cpp
)

mu_add_test(
    NAME texture_filtering_tests
    SOURCES
        test_texture_filtering.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Textures/TextureFiltering.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Textures/AssetPath.cpp
)

mu_add_test(
    NAME antialiasing_tests
    SOURCES
        test_antialiasing.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/Antialiasing.cpp
)

mu_add_test(
    NAME post_processing_tests
    SOURCES
        test_post_processing.cpp
        test_color_grading_table.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/PostProcessing/BloomSource.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/PostProcessing/ColorGradingTable.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/PostProcessing/SceneBoundary.cpp
    LINK_LIBS SDL3::SDL3
)

mu_add_test(
    NAME sdl_gpu_replay_state_tests
    SOURCES
        test_sdl_gpu_replay_state.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/SdlGpuReplayState.cpp
    LINK_LIBS SDL3::SDL3
)

mu_add_test(
    NAME range_allocator_tests
    SOURCES
        test_range_allocator.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/RangeAllocator.cpp
)

mu_add_test(
    NAME terrain_height_grid_tests
    SOURCES
        test_terrain_height_grid.cpp
)

# The decryption of version 14 BMD models, round-tripped against an encryption built the way the files are.
mu_add_test(
    NAME bmd_modulus_cipher_tests
    SOURCES
        test_bmd_modulus_cipher.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Models/BmdModulusCipher.cpp
    LINK_LIBS
        MuCryptopp
)

# The OZT reader of CGlobalBitmap::OpenTga, without the renderer. It also reads
# the OZT files shipped in src/bin/Data.
mu_add_test(
    NAME ozt_file_tests
    SOURCES
        test_ozt_file.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Textures/OztFile.cpp
)
target_compile_definitions(ozt_file_tests PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)

# The marker of upscaled art that CGlobalBitmap reads from OZJ and OZT files.
mu_add_test(
    NAME upscale_marker_tests
    SOURCES
        test_upscale_marker.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Textures/UpscaleMarker.cpp
)

# The texture packs whose files CGlobalBitmap reads before those in Data.
mu_add_test(
    NAME texture_pack_tests
    SOURCES
        test_texture_pack.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Textures/TexturePack.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Textures/TexturePackFiles.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Textures/AssetPath.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Sprites/GlobalBitmapPath.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Platform/PathResolve.cpp
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

# The playback rule: the plan per case, and BMD::Animation against MU's pose
# code on a synthetic skeleton. BMD::Animation comes with the client globals
# of Winmain.cpp.
mu_add_test(
    NAME animation_sampling_tests
    SOURCES
        test_animation_sampling.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Models/AnimationSampling.cpp
)

mu_add_test(
    NAME animation_playback_rule_tests
    SOURCES
        test_animation_playback_rule.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

# Monster models read from glTF binaries (Render/Models/Gltf): the Elite Yeti,
# Bull Fighter and Bali exported from their BMDs (data/Monster{14,01,33}.glb)
# against the BMDs themselves.
mu_add_test(
    NAME glb_model_tests
    SOURCES
        test_glb_model.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(glb_model_tests PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
    MU_TEST_GLB_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

mu_add_test(
    NAME dynamic_lights_tests
    SOURCES
        test_dynamic_lights.cpp
        test_dynamic_light_history.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Lighting/DynamicLightHistory.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Lighting/DynamicLightList.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Lighting/DynamicLightSelection.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Terrain/TerrainLightGrid.cpp
)

mu_add_test(
    NAME single_sample_twins_tests
    SOURCES test_single_sample_twins.cpp
)

mu_add_test(
    NAME scene_lighting_tests
    SOURCES
        test_scene_lighting.cpp
        test_world_pipeline_set.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/SceneLighting.cpp
)

mu_add_test(
    NAME light_mood_scales_tests
    SOURCES
        test_light_mood_scales.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Lighting/LightMoodScales.cpp
)

# The look profile's parameters: built-in values, CPU mirrors, range ends and uniform packing.
mu_add_test(
    NAME look_profile_tests
    SOURCES
        test_look_profile.cpp
        test_look_tuning.cpp
        test_look_preset.cpp
        test_tuning_controls.cpp
        test_tuning_session.cpp
        test_look_layers.cpp
    LINK_LIBS
        MuClient
)
# The player tuning tests copy the shipped look and mood files to check they stay untouched, and read
# the sources for the one place that applies a pending light mood strength.
target_compile_definitions(look_profile_tests PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
)

# The light mood against the real terrain light buffers; they come with the
# client globals of Winmain.cpp.
mu_add_test(
    NAME light_mood_terrain_tests
    SOURCES
        test_light_mood_terrain.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

add_test(
    NAME imgui_sdlgpu_backend_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SOURCE_DIR=${CMAKE_SOURCE_DIR}/src"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_imgui_sdlgpu_backend.cmake"
)

add_test(
    NAME particle_cull_keeps_depth_state
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_PARTICLE_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Effects/ZzzEffectParticle.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_particle_cull_state.cmake"
)

set(MU_EFFECTS_DIR "${CMAKE_SOURCE_DIR}/src/source/Render/Effects")
add_test(
    NAME create_functions_have_no_fps_gate
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_EFFECT_SOURCES=CreateEffect=${MU_EFFECTS_DIR}/ZzzEffect.cpp\;CreateJoint=${MU_EFFECTS_DIR}/ZzzEffectJoint.cpp\;CreateParticle=${MU_EFFECTS_DIR}/ZzzEffectParticle.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_create_has_no_fps_gate.cmake"
)

add_test(
    NAME fps_gates_are_not_nested
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_SOURCE_DIR=${CMAKE_SOURCE_DIR}/src/source"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_no_nested_fps_gate.cmake"
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
        "-DMU_DIAGNOSTICS_OVERLAY_SOURCE=${CMAKE_SOURCE_DIR}/src/source/UI/Diagnostics/DiagnosticsOverlay.cpp"
        "-DMU_DIAGNOSTICS_DOCUMENT=${CMAKE_SOURCE_DIR}/src/bin/Data/Interface/RmlUi/diagnostics.rml"
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
    NAME terrain_depth_state_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_TERRAIN_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Terrain/ZzzLodTerrain.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_terrain_depth_state.cmake"
)

add_test(
    NAME coin_heap_skin_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_BMD_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Models/ZzzBMD.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_coin_heap_skin.cmake"
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
