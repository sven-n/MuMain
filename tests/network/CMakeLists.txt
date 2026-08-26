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
        NAME login_diagnostics_contract
        COMMAND "${CMAKE_COMMAND}"
            "-DMU_LOGIN_SOURCE=${CMAKE_SOURCE_DIR}/ClientLibrary/ConnectionManager.ClientToServer.Custom.cs"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/test_login_diagnostics.cmake"
    )

    add_test(
        NAME managed_log_callback
        COMMAND "${CMAKE_COMMAND}" -E env --unset=Platform
            "${DOTNET_EXECUTABLE}" run
            --project ${CMAKE_CURRENT_SOURCE_DIR}/managed_log_self_check/ManagedLogSelfCheck.csproj
            --configuration Release
    )
    set_tests_properties(managed_log_callback PROPERTIES TIMEOUT 30)

    add_test(
        NAME outbound_flush_loop
        COMMAND "${CMAKE_COMMAND}" -E env --unset=Platform
            "${DOTNET_EXECUTABLE}" run
            --project ${CMAKE_CURRENT_SOURCE_DIR}/outbound_sender_self_check/OutboundSenderSelfCheck.csproj
            --configuration Release
    )
    set_tests_properties(outbound_flush_loop PROPERTIES TIMEOUT 30)
endif()
