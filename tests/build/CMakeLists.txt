# Build system validation tests for Story 1.1.2
# These use cmake -P (script mode) and shell scripts, not Catch2.

set(MUMAIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

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
