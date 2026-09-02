mu_add_test(
    NAME test_mu_timer
    SOURCES
        test_mu_timer.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_ground_truth
    SOURCES
        test_ground_truth.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_windows_console
    SOURCES
        test_windows_console.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_server_port
    SOURCES
        test_server_port.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_narrow_path
    SOURCES
        test_narrow_path.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_scene_state
    SOURCES
        test_scene_state.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_audio_playback
    SOURCES
        test_audio_playback.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(test_audio_playback PRIVATE
    MU_TEST_ASSET_SOURCE="${CMAKE_SOURCE_DIR}/src/bin"
)

mu_add_test(
    NAME test_platform_crypto
    SOURCES
        test_platform_crypto.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_mu_logger
    SOURCES
        test_mu_logger_static_init.cpp
        test_mu_logger_doctest.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_frame_profiler
    SOURCES
        test_frame_profiler.cpp
)

mu_add_test(
    NAME test_cpu_usage_math
    SOURCES
        test_cpu_usage_math.cpp
)

add_test(
    NAME msvc_runtime_dll_staging_contract
    COMMAND "${CMAKE_COMMAND}"
        "-DMU_CMAKE_SOURCE=${CMAKE_SOURCE_DIR}/src/CMakeLists.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/test_msvc_runtime_dll_staging.cmake"
)
