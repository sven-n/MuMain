# Build system validation tests
# These use cmake -P (script mode) and shell scripts, not Catch2.

set(MUMAIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

# ==========================================================================
# Story 1.1.2 - Linux CMake Toolchain
# ==========================================================================

# AC-1: Linux x64 toolchain file validation
add_test(
    NAME "AC-1:linux-toolchain-file"
    COMMAND ${CMAKE_COMMAND}
        -DTOOLCHAIN_FILE=${MUMAIN_SOURCE_DIR}/cmake/toolchains/linux-x64.cmake
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_linux_toolchain_file.cmake
)

# AC-2: CMakePresets.json Linux preset validation
add_test(
    NAME "AC-2:linux-presets"
    COMMAND ${CMAKE_COMMAND}
        -DPRESETS_FILE=${MUMAIN_SOURCE_DIR}/CMakePresets.json
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_linux_presets.cmake
)

# AC-3: cmake --preset linux-x64 configure succeeds (Linux hosts only)
add_test(
    NAME "AC-3:linux-configure"
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_linux_configure.sh
)

# ==========================================================================
# Story 1.3.1 - SDL3 Dependency Integration
# ==========================================================================

# AC-1: SDL3 FetchContent with pinned version in CMakeLists.txt
add_test(
    NAME "1.3.1-AC-1:sdl3-fetchcontent"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_sdl3_fetchcontent.cmake
)

# AC-3: SDL3 link visibility (MUPlatform + MURenderFX PRIVATE only)
add_test(
    NAME "1.3.1-AC-3:sdl3-link-visibility"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac3_sdl3_link_visibility.cmake
)

# AC-4: SDL3 headers not included in game logic directories
add_test(
    NAME "1.3.1-AC-4:sdl3-no-game-logic-includes"
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test_ac4_sdl3_no_game_logic_includes.sh
)

# AC-5: MU_ENABLE_SDL3 option for CI gating
add_test(
    NAME "1.3.1-AC-5:sdl3-ci-option"
    COMMAND ${CMAKE_COMMAND}
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/src/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac5_sdl3_ci_option.cmake
)

# ==========================================================================
# Story 3.1.1 - CMake RID Detection & .NET AOT Build Integration
# ==========================================================================

# AC-1 + AC-5: FindDotnetAOT.cmake RID detection + WSL detection logic
add_test(
    NAME "3.1.1-AC-1:dotnet-rid-detection"
    COMMAND ${CMAKE_COMMAND}
        -DMODULE_FILE=${MUMAIN_SOURCE_DIR}/cmake/FindDotnetAOT.cmake
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac1_dotnet_rid_detection.cmake
)

# AC-2: MU_DOTNET_LIB_EXT set per platform and passed to C++ compiler
add_test(
    NAME "3.1.1-AC-2:dotnet-lib-ext"
    COMMAND ${CMAKE_COMMAND}
        -DMODULE_FILE=${MUMAIN_SOURCE_DIR}/cmake/FindDotnetAOT.cmake
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac2_dotnet_lib_ext.cmake
)

# AC-6: Graceful failure when dotnet not found
add_test(
    NAME "3.1.1-AC-6:dotnet-graceful-failure"
    COMMAND ${CMAKE_COMMAND}
        -DMODULE_FILE=${MUMAIN_SOURCE_DIR}/cmake/FindDotnetAOT.cmake
        -DCMAKELISTS_FILE=${MUMAIN_SOURCE_DIR}/CMakeLists.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac6_dotnet_graceful_failure.cmake
)

# AC-STD-11: Flow code VS1-NET-CMAKE-RID traceability
add_test(
    NAME "3.1.1-AC-STD-11:flow-code-traceability"
    COMMAND ${CMAKE_COMMAND}
        -DMODULE_FILE=${MUMAIN_SOURCE_DIR}/cmake/FindDotnetAOT.cmake
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_ac_std11_flow_code_3_1_1.cmake
)
