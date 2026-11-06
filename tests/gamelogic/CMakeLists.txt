mu_add_test(
    NAME test_boid_steering
    SOURCES
        test_boid_steering.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_bird_landing
    SOURCES
        test_bird_landing.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_head_movement
    SOURCES
        test_head_movement.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_earthquake_falloff
    SOURCES
        test_earthquake_falloff.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_effect_draw
    SOURCES
        test_effect_draw.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_character_effects_phase
    SOURCES
        test_character_effects_phase.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

# The draw guard on the gameplay random sequences: no control socket code, so it builds and runs without the socket.
mu_add_test(
    NAME test_draw_random_guard
    SOURCES
        test_draw_random_guard.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_repeated_draws
    SOURCES
        test_repeated_draws.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_reference_frame_steps
    SOURCES
        test_reference_frame_steps.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_direction_turning
    SOURCES
        test_direction_turning.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_personal_shop_currency
    SOURCES
        test_personal_shop_currency.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_helper_attack_rotation
    SOURCES
        test_helper_attack_rotation.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS MuClient
)
if(WIN32 AND TARGET ClientLibrary)
    target_compile_definitions(test_helper_attack_rotation PRIVATE
        MU_TEST_COMBAT_AOT_TRANSPORT=1
        MU_BIN_DIR="${CMAKE_SOURCE_DIR}/src/bin"
    )
    target_link_libraries(test_helper_attack_rotation PRIVATE ws2_32)
    add_dependencies(test_helper_attack_rotation ClientLibrary)
    add_custom_command(TARGET test_helper_attack_rotation PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/src/MUnique.Client.Library.dll"
            "$<TARGET_FILE_DIR:test_helper_attack_rotation>/MUnique.Client.Library.dll"
        COMMENT "Staging ClientLibrary beside the combat test"
        VERBATIM
    )
endif()

mu_add_test(
    NAME test_ancient_set_options
    SOURCES
        test_ancient_set_options.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_helper_actions
    SOURCES
        test_helper_actions.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS MuClient
)
