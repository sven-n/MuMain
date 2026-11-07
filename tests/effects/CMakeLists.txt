mu_add_test(
    NAME test_particle_draw_order
    SOURCES
        test_particle_draw_order.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Effects/ParticleDrawOrder.cpp
)

# The shipped effect catalogue and the registry built from it, for test_effect_creation and test_effect_types.
mu_add_test_objects(NAME mu_test_effect_data SOURCES EffectTestData.cpp LINK_LIBS mu_test_main MuClient)
target_compile_definitions(mu_test_effect_data PRIVATE MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data")

# Creates effects through CreateEffect and records what changes: the effect
# recorder of the effect catalogue (D40). A binary of its own, because the
# recorder takes over the effect pools, the registry and the hero; the effect
# code needs the client globals of Winmain.cpp.
mu_add_test(
    NAME test_effect_creation
    SOURCES
        EffectRecorder.cpp
        test_effect_creation.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
        mu_test_effect_data
)
target_compile_definitions(test_effect_creation PRIVATE
    MU_EFFECT_CREATION_BASELINE="${CMAKE_CURRENT_SOURCE_DIR}/baseline/EffectCreation.txt"
)
