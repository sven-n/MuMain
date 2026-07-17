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
