# bmdconv - converts between the engine's BMD models and Valve SMD text files (the
# format Blender Source Tools reads and writes). It links the engine library on demand
# so the game's own SMD parser (Render/Models/SMD.cpp) and BMD serializer
# (BMD::Open2 / BMD::Save2) do the actual work; the tool adds validation, the reverse
# BMD -> SMD direction and a command-line interface. Usage: docs/asset-pipeline.md.

add_library(bmdconv_lib STATIC
    BmdSmdConverter.cpp
    BmdSmdConverter.h
    SmdValidator.cpp
    SmdValidator.h
)
target_include_directories(bmdconv_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
# Raw MuClient (not the WHOLE_ARCHIVE alias): only the model/parser objects are needed,
# the same way the unit tests link the engine.
target_link_libraries(bmdconv_lib PUBLIC MuClient)
if(MSVC)
    target_compile_options(bmdconv_lib PUBLIC /utf-8)
endif()

# The engine objects reference the bootstrap globals (window handle, fonts, options)
# that live in Winmain.cpp, which is kept out of the library on purpose; compile it into
# the executable the same way tests/core does for test_audio_playback.
set(MU_BOOTSTRAP_SOURCE "${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp")

add_executable(bmdconv main.cpp "${MU_BOOTSTRAP_SOURCE}")
target_link_libraries(bmdconv PRIVATE bmdconv_lib)
