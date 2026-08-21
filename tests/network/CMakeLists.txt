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

mu_add_test(
    NAME test_chat_command_catalog
    SOURCES
        test_chat_command_catalog.cpp
        ${CMAKE_SOURCE_DIR}/src/source/GameLogic/Commands/ChatCommandCatalog.cpp
)

add_test(
    NAME packet_binding_resolution
    COMMAND ${CMAKE_COMMAND}
        -DBINDINGS=${CMAKE_SOURCE_DIR}/src/source/Dotnet/PacketBindings_ClientToServer.h
        -DCONNECTION=${CMAKE_SOURCE_DIR}/src/source/Dotnet/Connection.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_packet_binding_resolution.cmake
)

add_test(
    NAME stale_connection_packets_are_discarded
    COMMAND ${CMAKE_COMMAND}
        -DHEADER_FILE=${CMAKE_SOURCE_DIR}/src/source/Network/Server/WSclient.h
        -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/src/source/Network/Server/WSclient.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_stale_connection_packets.cmake
)
