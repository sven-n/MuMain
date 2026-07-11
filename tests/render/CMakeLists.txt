mu_add_test(
    NAME frame_pixel_readback_tests
    SOURCES
        test_frame_pixel_readback.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Renderer/FramePixelReadback.cpp
)
