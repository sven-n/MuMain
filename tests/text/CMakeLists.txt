mu_add_test(
    NAME test_text_line_wrap
    SOURCES
        test_sdl_ttf_color.cpp
        test_text_line_wrap.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Text/TextLineWrap.cpp
)

mu_add_test(
    NAME sdl_ttf_text_cache_tests
    SOURCES
        test_sdl_ttf_text_cache.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Text/SdlTtfTextCache.cpp
    LINK_LIBS SDL3_ttf::SDL3_ttf SDL3::SDL3
)
target_compile_definitions(sdl_ttf_text_cache_tests PRIVATE
    MU_TEST_FONT_PATH="${CMAKE_SOURCE_DIR}/src/bin/fonts/DejaVuSans.ttf"
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
