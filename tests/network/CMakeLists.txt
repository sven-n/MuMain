mu_add_test(
    NAME test_server_config_validation
    SOURCES
        test_server_config_validation.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Data/GameConfig/GameConfigValidation.cpp
)

mu_add_test(
    NAME test_connection_library_load
    SOURCES
        test_connection_library_load.cpp
    LINK_LIBS
        MuClient
)
