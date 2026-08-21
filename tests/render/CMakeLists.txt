mu_add_test(
    NAME frame_pixel_readback_tests
    SOURCES
        test_frame_pixel_readback.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/DiagnosticFrameCaptureSchedule.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/FramePixelReadback.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/SdlGpuPixelFormat.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Scenes/ScreenshotCaptureState.cpp
    LINK_LIBS
        SDL3::SDL3
)

mu_add_test(
    NAME gpu_skinning_contract_tests
    SOURCES
        test_gpu_skinning_contract.cpp
)

add_test(
    NAME sdl_gpu_cull_face_pipeline
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_cull_face_pipeline.cmake
)

add_test(
    NAME sdl_gpu_reconnect_capture_target
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_reconnect_capture_target.cmake
)

add_test(
    NAME sdl_gpu_vertex_overflow_guard
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_vertex_overflow_guard.cmake
)

add_test(
    NAME sdl_gpu_bone_skinning_path
    COMMAND ${CMAKE_COMMAND}
        -DRENDERER_HEADER=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRenderer.h
        -DRENDERER_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp
        -DBMD_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Models/ZzzBMD.cpp
        -DSHADER_SOURCE=${CMAKE_SOURCE_DIR}/src/shaders/skinned_textured.vert.hlsl
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_gpu_skinning_path.cmake
)

add_test(
    NAME camera_projection_updates_renderer_matrix
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/src/source/Camera/CameraProjection.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_camera_projection_path.cmake
)

add_test(
    NAME sdl_gpu_vsync_frame_pacing
    COMMAND ${CMAKE_COMMAND}
        -DRENDERER_HEADER=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRenderer.h
        -DRENDERER_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp
        -DRENDER_UTIL_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Textures/ZzzOpenglUtil.cpp
        -DWINMAIN_SOURCE=${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
        -DCONSOLE_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Core/Utilities/Log/muConsoleDebug.cpp
        -DSRC_CMAKE=${CMAKE_SOURCE_DIR}/src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_vsync_frame_pacing.cmake
)

add_test(
    NAME terrain_texture_uv_scale
    COMMAND ${CMAKE_COMMAND}
        -DTERRAIN_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Terrain/ZzzLodTerrain.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_terrain_texture_uv_scale.cmake
)

add_test(
    NAME sdl_gpu_frame_stats
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_sdl_gpu_frame_stats.cmake
)

add_test(
    NAME glp29_sprite_path
    COMMAND ${CMAKE_COMMAND}
        -DRENDER_UTIL_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Textures/ZzzOpenglUtil.cpp
        -DRENDERER_SOURCE=${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/MuRendererSDLGpu.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_glp29_sprite_path.cmake
)

add_test(
    NAME global_bitmap_empty_shutdown_guard
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/src/source/Render/Sprites/GlobalBitmap.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_global_bitmap_empty_shutdown_guard.cmake
)

add_test(
    NAME legacy_ui_line_color_state
    COMMAND ${CMAKE_COMMAND}
        -DUI_SOURCE_FILE=${CMAKE_SOURCE_DIR}/src/source/UI/Legacy/UIWindows.cpp
        -DRENDER_SOURCE_FILE=${CMAKE_SOURCE_DIR}/src/source/Render/Textures/ZzzOpenglUtil.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_legacy_ui_line_color_state.cmake
)
