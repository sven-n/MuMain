mu_add_test(
    NAME test_server_config_validation
    SOURCES
        test_server_config_validation.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Data/GameConfig/GameConfigValidation.cpp
)

mu_add_test(
    NAME test_byte_order
    SOURCES
        test_byte_order.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_fixed_c1_packet
    SOURCES
        test_fixed_c1_packet.cpp
)

mu_add_test(
    NAME test_guild_relationship_request
    SOURCES
        test_guild_relationship_request.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Network/Server/GuildRelationshipRequest.cpp
)

mu_add_test(
    NAME test_guild_relationship_result
    SOURCES
        test_guild_relationship_result.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Network/Server/GuildRelationshipResult.cpp
)

mu_add_test(
    NAME test_guild_relationship_result_text
    SOURCES
        test_guild_relationship_result_text.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_remove_alliance_guild_result
    SOURCES
        test_remove_alliance_guild_result.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Network/Server/RemoveAllianceGuildResult.cpp
)

mu_add_test(
    NAME test_illusion_temple_result
    SOURCES
        test_illusion_temple_result.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Network/Server/IllusionTempleResult.cpp
)

mu_add_test(
    NAME test_transform_viewport_entry
    SOURCES
        test_transform_viewport_entry.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_transform_appearance
    SOURCES
        test_transform_appearance.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS MuClient
)
target_compile_definitions(test_transform_appearance PRIVATE MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data")

mu_add_test(
    NAME test_connection_library_load
    SOURCES
        test_connection_library_load.cpp
        ${MU_MANAGED_SYMBOL_ADAPTER}
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_managed_symbols
    SOURCES
        test_managed_symbols.cpp
    LINK_LIBS
        MuClient
)

# A target with an adapter of its own: no Dotnet/SymbolSources/ file and no Winmain.cpp, which starts the bridge from
# the game's adapter.
mu_add_test(
    NAME test_managed_symbol_target
    SOURCES
        test_managed_symbol_target.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_endpoint_for_log
    SOURCES
        test_endpoint_for_log.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_login_scene_reconnect
    SOURCES
        test_login_scene_reconnect.cpp
    LINK_LIBS
        MuClient
)

# The game's side of that reconnect reads the login scene's windows, which need
# the client globals of Winmain.cpp.
mu_add_test(
    NAME test_game_login_scene_link
    SOURCES
        test_game_login_scene_link.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_connection_lifecycle
    SOURCES
        test_connection_lifecycle.cpp
)

mu_add_test(
    NAME test_incoming_packet_queue
    SOURCES
        test_incoming_packet_queue.cpp
    LINK_LIBS
        MuClient
)

# Needs the managed library beside the test, which only the Windows build stages for tests.
if(WIN32 AND TARGET ClientLibrary)
    mu_add_test(
        NAME test_connection_open
        SOURCES
            test_connection_open.cpp
            ${MU_MANAGED_SYMBOL_ADAPTER}
        LINK_LIBS
            MuClient
    )
    target_link_libraries(test_connection_open PRIVATE ws2_32)
    add_dependencies(test_connection_open ClientLibrary)
    add_custom_command(TARGET test_connection_open PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/src/MUnique.Client.Library.dll"
            "$<TARGET_FILE_DIR:test_connection_open>/MUnique.Client.Library.dll"
        COMMENT "Staging ClientLibrary beside the connection test"
        VERBATIM
    )
endif()

find_package(Threads REQUIRED)
mu_add_test(
    NAME test_handle_registry
    SOURCES
        test_handle_registry.cpp
    LINK_LIBS
        Threads::Threads
)

# Control socket tests only exist in builds that enable it. Without it, the
# control_socket_leak test proves that none of its sources were compiled into
# the client (same mechanics as tests/editor's editor_leak).
if(ENABLE_CONTROL_SOCKET)
    mu_add_test(
        NAME test_control_protocol
        SOURCES
            test_control_protocol.cpp
        LINK_LIBS
            MuClient
    )

    # The target lookup reads the character table, which needs the client
    # globals of Winmain.cpp.
    mu_add_test(
        NAME test_control_targets
        SOURCES
            test_control_targets.cpp
            ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
        LINK_LIBS
            MuClient
    )

    # The kind names come from the object code, which needs the client
    # globals of Winmain.cpp.
    mu_add_test(
        NAME test_control_events
        SOURCES
            test_control_events.cpp
            ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
        LINK_LIBS
            MuClient
    )
else()
    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/muclient_sources.txt"
        CONTENT "$<JOIN:$<TARGET_PROPERTY:MuClient,SOURCES>,\n>")

    add_test(
        NAME control_socket_leak
        COMMAND ${CMAKE_COMMAND}
            "-DSOURCES_FILE=${CMAKE_CURRENT_BINARY_DIR}/muclient_sources.txt"
            "-DCONTROL_DIR=${PROJECT_SOURCE_DIR}/src/source/App/Control"
            "-DTRANSPORT_FILE=${PROJECT_SOURCE_DIR}/src/source/Core/Platform/LocalSocket.cpp"
            "-DINJECTOR_FILE=${PROJECT_SOURCE_DIR}/src/source/Core/Input/SyntheticInput.cpp"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/test_control_leak.cmake")
endif()

mu_add_test(
    NAME test_chat_command_catalog
    SOURCES
        test_chat_command_catalog.cpp
        ${CMAKE_SOURCE_DIR}/src/source/GameLogic/Commands/ChatCommandCatalog.cpp
)

if(DOTNET_EXECUTABLE)
    # Each build folder builds the self-checks into its own folder: two build folders testing at once would
    # otherwise write the same bin/ and obj/ next to the projects.
    set(MU_DOTNET_SELF_CHECK_ARTIFACTS "${CMAKE_CURRENT_BINARY_DIR}/dotnet-self-checks")

    add_test(
        NAME dotnet_binding_generator
        COMMAND "${CMAKE_COMMAND}" -E env --unset=Platform
            "${DOTNET_EXECUTABLE}" run
            --project ${CMAKE_CURRENT_SOURCE_DIR}/binding_generator_self_check/BindingGeneratorSelfCheck.csproj
            --configuration Release
            --artifacts-path ${MU_DOTNET_SELF_CHECK_ARTIFACTS}
            -- ${CMAKE_SOURCE_DIR}/ClientLibrary/GenerateBindingsHeader.xslt
               ${CMAKE_SOURCE_DIR}/ClientLibrary/GenerateFunctions.xslt
               ${CMAKE_SOURCE_DIR}/src/source/Dotnet
    )

    add_test(
        NAME managed_log_callback
        COMMAND "${CMAKE_COMMAND}" -E env --unset=Platform
            "${DOTNET_EXECUTABLE}" run
            --project ${CMAKE_CURRENT_SOURCE_DIR}/managed_log_self_check/ManagedLogSelfCheck.csproj
            --configuration Release
            --artifacts-path ${MU_DOTNET_SELF_CHECK_ARTIFACTS}
    )
    set_tests_properties(managed_log_callback PROPERTIES TIMEOUT 30)

    add_test(
        NAME connect_failure_description
        COMMAND "${CMAKE_COMMAND}" -E env --unset=Platform
            "${DOTNET_EXECUTABLE}" run
            --project ${CMAKE_CURRENT_SOURCE_DIR}/connect_failure_self_check/ConnectFailureSelfCheck.csproj
            --configuration Release
            --artifacts-path ${MU_DOTNET_SELF_CHECK_ARTIFACTS}
    )
    set_tests_properties(connect_failure_description PROPERTIES TIMEOUT 30)

    add_test(
        NAME deferred_tcp_pipe
        COMMAND "${CMAKE_COMMAND}" -E env --unset=Platform
            "${DOTNET_EXECUTABLE}" run
            --project ${CMAKE_CURRENT_SOURCE_DIR}/deferred_tcp_pipe_self_check/DeferredTcpPipeSelfCheck.csproj
            --configuration Release
            --artifacts-path ${MU_DOTNET_SELF_CHECK_ARTIFACTS}
    )
    set_tests_properties(deferred_tcp_pipe PROPERTIES TIMEOUT 60)

    add_test(
        NAME outbound_flush_loop
        COMMAND "${CMAKE_COMMAND}" -E env --unset=Platform
            "${DOTNET_EXECUTABLE}" run
            --project ${CMAKE_CURRENT_SOURCE_DIR}/outbound_sender_self_check/OutboundSenderSelfCheck.csproj
            --configuration Release
            --artifacts-path ${MU_DOTNET_SELF_CHECK_ARTIFACTS}
    )
    set_tests_properties(outbound_flush_loop PROPERTIES TIMEOUT 30)
endif()

mu_add_test(
    NAME test_position_glide
    SOURCES
        test_position_glide.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS MuClient
)
