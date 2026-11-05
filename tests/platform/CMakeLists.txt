mu_add_test(
    NAME sdl_window_flags_tests
    SOURCES
        test_sdl_window_flags.cpp
    LINK_LIBS
        SDL3::SDL3
)

mu_add_test(
    NAME process_locale_tests
    SOURCES
        test_process_locale.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Platform/ProcessLocale.cpp
)

mu_add_test(
    NAME secure_crt_tests
    SOURCES
        test_secure_crt.cpp
)

# The .ini profile functions: Windows' own, and the shim (WinIni.cpp) on the other platforms.
mu_add_test(
    NAME win_ini_tests
    SOURCES
        test_win_ini.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Platform/WinIni.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Platform/WinNls.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Platform/PathResolve.cpp
)

# Font sizing uses the real UI scaling layer. Supply its application-owned globals and
# preferences through the same entry-point glue as ui_scaling_tests, including editor builds.
mu_add_test(
    NAME bundled_font_role_tests
    SOURCES
        test_bundled_font_roles.cpp
        test_translation_glyph_coverage.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS MuClient SDL3_ttf::SDL3_ttf SDL3::SDL3
)
target_compile_definitions(bundled_font_role_tests PRIVATE
    MU_TEST_ASSET_SOURCE="${CMAKE_SOURCE_DIR}/src/bin"
    MU_TEST_LOCALIZATION_DIR="${CMAKE_SOURCE_DIR}/src/Localization"
)

add_test(
    NAME bundled_font_assets
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_ASSET_SOURCE=${CMAKE_SOURCE_DIR}/src/bin"
        "-DMU_COPY_SCRIPT=${CMAKE_SOURCE_DIR}/cmake/CopyRuntimeAssets.cmake"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_bundled_font_assets.cmake"
)
