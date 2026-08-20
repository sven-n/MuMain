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
    NAME test_platform_crypto
    SOURCES
        test_platform_crypto.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_mu_logger
    SOURCES
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
