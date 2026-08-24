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
    NAME quad_topology_tests
    SOURCES
        test_quad_topology.cpp
)

mu_add_test(
    NAME aura_joint_lifecycle_tests
    SOURCES
        test_aura_joint_lifecycle.cpp
)
