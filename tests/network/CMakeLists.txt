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

if(DOTNET_EXECUTABLE)
    add_test(
        NAME outbound_flush_loop
        COMMAND "${DOTNET_EXECUTABLE}" run
            --project ${CMAKE_CURRENT_SOURCE_DIR}/outbound_sender_self_check/OutboundSenderSelfCheck.csproj
            --configuration Release
    )
    set_tests_properties(outbound_flush_loop PROPERTIES TIMEOUT 30)
endif()
