mu_add_test(
    NAME test_particle_draw_order
    SOURCES
        test_particle_draw_order.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Render/Effects/ParticleDrawOrder.cpp
)

# Creates effects through CreateEffect and records what changes: the effect
# recorder of the effect catalogue (D40). A binary of its own, because the
# recorder takes over the effect pools, the registry and the hero; the effect
# code needs the client globals of Winmain.cpp.
mu_add_test(
    NAME test_effect_creation
    SOURCES
        EffectRecorder.cpp
        EffectTestData.cpp
        test_effect_creation.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS
        MuClient
)
target_compile_definitions(test_effect_creation PRIVATE
    MU_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/bin/Data"
    MU_EFFECT_RECORDINGS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/recordings"
)
