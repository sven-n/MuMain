# Flow Code: VS0-QUAL-WIN32CLEAN-STRINCLUDE
# Story 7.6.2: Win32 String Conversion and Include Guard Cleanup
# AC-7: RenderFX/MuRendererSDLGpu.cpp SDL3 includes are unconditional;
#        #ifdef MU_ENABLE_SDL3 guard around include block removed.
#
# Rationale: MuRendererSDLGpu.cpp is already CMake-excluded when MU_ENABLE_SDL3 is OFF,
# so the preprocessor guard around SDL3 #include lines is redundant and creates an
# asymmetry — the TU compiles but SDL3 types are missing, causing confusing errors.
# The correct model: the CMake-level gate determines whether the TU is compiled at all.
#
# RED PHASE: Fails until Task 6 (MuRendererSDLGpu.cpp) is complete.

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SOURCE_FILE)
    message(FATAL_ERROR "AC-7: SOURCE_FILE variable not defined")
endif()

if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "AC-7: MuRendererSDLGpu.cpp not found at: ${SOURCE_FILE}")
endif()

file(READ "${SOURCE_FILE}" content)

# Check 1: SDL3/SDL.h or SDL3/SDL_gpu.h MUST appear as an include
string(REGEX MATCH "#include[ \t]*[<\"]SDL3/SDL[^\"]*[>\"]" _match_sdl3 "${content}")
if(NOT _match_sdl3)
    message(FATAL_ERROR "AC-7 FAILED: MuRendererSDLGpu.cpp does not include any SDL3 header — expected #include <SDL3/SDL.h> or similar")
endif()

# Check 2: SDL3 includes must NOT be wrapped in #ifdef MU_ENABLE_SDL3 ... #endif
# Strategy: check that the SDL3 include is NOT immediately preceded by #ifdef MU_ENABLE_SDL3
string(REGEX MATCH "#ifdef MU_ENABLE_SDL3[^\n]*\n[^\n]*SDL3/" _match_guarded_include "${content}")
if(_match_guarded_include)
    message(FATAL_ERROR "AC-7 FAILED: MuRendererSDLGpu.cpp still wraps SDL3 includes in #ifdef MU_ENABLE_SDL3 — remove this guard (CMake handles TU exclusion)")
endif()

# Check 3: SDL3 include must appear near top of file (within first 50 lines)
string(SUBSTRING "${content}" 0 2000 header_section)
string(REGEX MATCH "#include[ \t]*[<\"]SDL3/" _match_early_sdl3 "${header_section}")
if(NOT _match_early_sdl3)
    message(FATAL_ERROR "AC-7 FAILED: SDL3 include not found in first ~50 lines of MuRendererSDLGpu.cpp — expected unconditional includes at top of file")
endif()

message(STATUS "AC-7 PASSED: MuRendererSDLGpu.cpp has unconditional SDL3 includes (no #ifdef MU_ENABLE_SDL3 around includes)")
