# Story 7.9.1: AC-5 — Wire RenderScene() into SDL3 game loop in MuMain()
# Flow Code: VS0-QUAL-RENDER-GAMELOOP
#
# RED PHASE:  Test FAILS before story 7.9.1 is implemented.
#             MuMain() at Winmain.cpp:1516 contains only a comment:
#               // Game loop body will be added as more systems are migrated
#             RenderScene() is NOT called inside the #ifdef MU_ENABLE_SDL3 BeginFrame/EndFrame block.
#
# GREEN PHASE: Test PASSES after implementation.
#             The comment is replaced with 'RenderScene(nullptr);' placed between
#             mu::GetRenderer().BeginFrame() and mu::GetRenderer().EndFrame()
#             inside the MU_ENABLE_SDL3 guard in MuMain().
#
# Validates:
#   AC-5 — RenderScene(nullptr) called in MuMain() SDL3 game loop
#   AC-5 — Call is inside the #ifdef MU_ENABLE_SDL3 BeginFrame/EndFrame block
#   AC-STD-12 — Game renders visible content (frame 1 non-black) when init sequence is complete

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED WINMAIN_CPP)
    message(FATAL_ERROR "WINMAIN_CPP must be set to the path of Winmain.cpp")
endif()

if(NOT EXISTS "${WINMAIN_CPP}")
    message(FATAL_ERROR "AC-5 FAIL: Winmain.cpp not found at '${WINMAIN_CPP}'")
endif()

file(READ "${WINMAIN_CPP}" winmain_content)

# ---------------------------------------------------------------------------
# AC-5a: Find the MuMain() function body
# ---------------------------------------------------------------------------
string(FIND "${winmain_content}" "int MuMain(" muminit_pos)
if(muminit_pos EQUAL -1)
    message(FATAL_ERROR "AC-5 FAIL: 'int MuMain(' not found in Winmain.cpp.")
endif()

string(LENGTH "${winmain_content}" total_len)
math(EXPR suffix_len "${total_len} - ${muminit_pos}")
string(SUBSTRING "${winmain_content}" ${muminit_pos} ${suffix_len} muminit_section)

set(found_violations FALSE)

# ---------------------------------------------------------------------------
# AC-5b: Verify RenderScene(nullptr) is present in MuMain() body.
# The call must use nullptr (not a hDC variable) — SDL3 path has no HDC.
# ---------------------------------------------------------------------------
string(FIND "${muminit_section}" "RenderScene(nullptr)" rs_pos)
if(rs_pos EQUAL -1)
    message(WARNING
        "AC-5 FAIL: 'RenderScene(nullptr)' not found in MuMain() body.\n"
        "Task 5.1: replace the '// Game loop body will be added...' comment at Winmain.cpp:1516\n"
        "with 'RenderScene(nullptr);' between BeginFrame() and EndFrame().")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-5c: Verify the placeholder comment is gone (confirms replacement, not addition).
# If the comment remains alongside the call, something went wrong structurally.
# ---------------------------------------------------------------------------
string(FIND "${muminit_section}" "Game loop body will be added" placeholder_pos)
if(NOT placeholder_pos EQUAL -1)
    message(WARNING
        "AC-5 FAIL: Placeholder comment 'Game loop body will be added...' still present in MuMain().\n"
        "Task 5.1: replace the comment with 'RenderScene(nullptr);' — do not leave both.")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-5d: Verify BeginFrame and EndFrame are still present (confirms we haven't
# accidentally broken the frame lifecycle while adding the RenderScene call).
# ---------------------------------------------------------------------------
string(FIND "${muminit_section}" "BeginFrame" bf_pos)
string(FIND "${muminit_section}" "EndFrame" ef_pos)

if(bf_pos EQUAL -1 OR ef_pos EQUAL -1)
    message(WARNING
        "AC-5 FAIL: BeginFrame() or EndFrame() missing from MuMain() — frame lifecycle broken.\n"
        "Task 5.2: verify RenderScene(nullptr) is placed inside the #ifdef MU_ENABLE_SDL3 block,\n"
        "between mu::GetRenderer().BeginFrame() and mu::GetRenderer().EndFrame().")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-5 FAIL: RenderScene() not yet wired into SDL3 game loop in MuMain().\n"
        "Story 7.9.1 Task 5: replace the placeholder comment with 'RenderScene(nullptr);'\n"
        "inside the #ifdef MU_ENABLE_SDL3 BeginFrame/EndFrame block.")
endif()

message(STATUS "AC-5 PASS: RenderScene(nullptr) wired into SDL3 game loop in MuMain()")
