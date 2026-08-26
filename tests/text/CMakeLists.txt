mu_add_test(
    NAME test_text_line_wrap
    SOURCES
        test_sdl_ttf_color.cpp
        test_text_line_wrap.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Text/TextLineWrap.cpp
)

FetchContent_GetProperties(SDL3_ttf SOURCE_DIR SDL3_TTF_SOURCE)
add_test(
    NAME sdl_ttf_fetchcontent_patch
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_ROOT_CMAKE=${CMAKE_SOURCE_DIR}/CMakeLists.txt"
        "-DSDL3_TTF_SOURCE=${SDL3_TTF_SOURCE}"
        "-DMU_PROPERTY_HEADER=${CMAKE_SOURCE_DIR}/src/source/Render/Text/SdlTtfGpuTextProperties.h"
        "-DMU_PATCH=${CMAKE_SOURCE_DIR}/cmake/patches/sdl_ttf-3.2.2-batch-glyph-uploads.patch"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_sdl_ttf_fetchcontent_patch.cmake"
)
