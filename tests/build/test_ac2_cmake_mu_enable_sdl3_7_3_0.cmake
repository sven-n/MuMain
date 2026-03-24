# Story 7.3.0: AC-2 — MU_ENABLE_SDL3 defined at project scope in src/CMakeLists.txt
# Flow Code: VS0-QUAL-BUILDCOMPAT-MACOS
#
# RED PHASE:  Test FAILS before story 7.3.0 is implemented.
#             MU_ENABLE_SDL3 is defined ONLY for MUPlatform target (PRIVATE),
#             so MUCore/MUData/MURenderFX/MUProtocol/MUAudio never receive the flag.
#
# GREEN PHASE: Test PASSES after fix.
#             add_compile_definitions(MU_ENABLE_SDL3) appears inside if(MU_ENABLE_SDL3)
#             block at project scope, propagating the flag to ALL targets.
#
# Validates:
#   AC-2 — add_compile_definitions(MU_ENABLE_SDL3) at project scope in src/CMakeLists.txt

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CMAKELISTS_FILE)
    message(FATAL_ERROR "CMAKELISTS_FILE must be set to the path of MuMain/src/CMakeLists.txt")
endif()

if(NOT EXISTS "${CMAKELISTS_FILE}")
    message(FATAL_ERROR "AC-2 FAIL: src/CMakeLists.txt not found at '${CMAKELISTS_FILE}'")
endif()

file(READ "${CMAKELISTS_FILE}" cmake_content)

# Check for add_compile_definitions(MU_ENABLE_SDL3) — project-scope propagation
string(FIND "${cmake_content}" "add_compile_definitions(MU_ENABLE_SDL3)" add_def_pos)
if(add_def_pos EQUAL -1)
    message(FATAL_ERROR
        "AC-2 FAIL: 'add_compile_definitions(MU_ENABLE_SDL3)' not found in src/CMakeLists.txt.\n"
        "Fix: add 'add_compile_definitions(MU_ENABLE_SDL3)' inside the if(MU_ENABLE_SDL3) block\n"
        "to propagate the flag to all CMake targets (MUCore, MUData, MURenderFX, MUProtocol, MUAudio).")
endif()

# Verify the project-scope definition appears WITHIN the if(MU_ENABLE_SDL3) block
# (not as an unconditional add_compile_definitions call)
string(FIND "${cmake_content}" "if(MU_ENABLE_SDL3)" sdl3_block_pos)
if(sdl3_block_pos EQUAL -1)
    message(FATAL_ERROR "AC-2 FAIL: if(MU_ENABLE_SDL3) block not found in src/CMakeLists.txt")
endif()

# The add_compile_definitions must appear after the if(MU_ENABLE_SDL3) opening
if(add_def_pos LESS sdl3_block_pos)
    message(FATAL_ERROR
        "AC-2 FAIL: add_compile_definitions(MU_ENABLE_SDL3) appears BEFORE the if(MU_ENABLE_SDL3) block.\n"
        "It must appear inside the if(MU_ENABLE_SDL3) conditional to avoid unconditional SDL3 enabling.")
endif()

message(STATUS "AC-2 PASS: add_compile_definitions(MU_ENABLE_SDL3) found at project scope in if(MU_ENABLE_SDL3) block")
