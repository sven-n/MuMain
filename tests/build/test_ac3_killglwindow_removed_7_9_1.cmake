# Story 7.9.1: AC-3 — Replace KillGLWindow() in RenderScene() with Destroy = true
# Flow Code: VS0-QUAL-RENDER-GAMELOOP
#
# RED PHASE:  Test FAILS before story 7.9.1 is implemented.
#             KillGLWindow() is called directly at SceneManager.cpp:1024
#             (inside RenderScene(), within the 'if (g_iNoMouseTime > 31)' guard).
#             KillGLWindow is NOT shimmed in PlatformCompat.h for non-Windows —
#             this is a compile/link error on macOS when RenderScene() is called.
#
# GREEN PHASE: Test PASSES after implementation.
#             KillGLWindow() call at SceneManager.cpp:1024 is replaced with 'Destroy = true'.
#             The extern declaration in SceneCore.cpp:125 may remain (used by ZzzTexture.cpp
#             under #ifdef MU_USE_OPENGL_BACKEND — Windows-only build path).
#
# Validates:
#   AC-3 — KillGLWindow() direct call removed from SceneManager.cpp (RenderScene path)
#   AC-3 — Destroy = true replaces the call, using the SDL3 graceful exit signal

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SCENE_MANAGER_CPP)
    message(FATAL_ERROR "SCENE_MANAGER_CPP must be set to the path of SceneManager.cpp")
endif()

if(NOT EXISTS "${SCENE_MANAGER_CPP}")
    message(FATAL_ERROR "AC-3 FAIL: SceneManager.cpp not found at '${SCENE_MANAGER_CPP}'")
endif()

file(READ "${SCENE_MANAGER_CPP}" scene_manager_content)

# ---------------------------------------------------------------------------
# AC-3: Check that KillGLWindow() is not called in SceneManager.cpp.
#
# Strategy: search the entire file for a KillGLWindow() call expression.
# The extern declaration in SceneCore.cpp is allowed to remain, but SceneManager.cpp
# must not contain any direct invocation of KillGLWindow().
#
# An 'extern' forward declaration does NOT appear in SceneManager.cpp (it's in SceneCore.cpp),
# so any occurrence of KillGLWindow in SceneManager.cpp is an unguarded call.
# ---------------------------------------------------------------------------
set(found_violations FALSE)

string(FIND "${scene_manager_content}" "KillGLWindow" kgw_pos)
if(NOT kgw_pos EQUAL -1)
    message(WARNING
        "AC-3 FAIL: 'KillGLWindow' still referenced in SceneManager.cpp at position ${kgw_pos}.\n"
        "Replace 'KillGLWindow();' at SceneManager.cpp:1024 with 'Destroy = true;'.\n"
        "Destroy signals the SDL3 event loop (while (!Destroy)) to exit gracefully —\n"
        "identical to the SDL_EVENT_QUIT path in PollEvents().")
    set(found_violations TRUE)
endif()

# Also verify 'Destroy = true' is now present in SceneManager.cpp (confirms the replacement)
string(FIND "${scene_manager_content}" "Destroy = true" destroy_pos)
if(destroy_pos EQUAL -1)
    message(WARNING
        "AC-3 FAIL: 'Destroy = true' not found in SceneManager.cpp.\n"
        "Task 3.1: replace KillGLWindow() with 'Destroy = true' at SceneManager.cpp:1024.")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-3 FAIL: KillGLWindow() not yet replaced in SceneManager.cpp.\n"
        "Story 7.9.1 Task 3: replace KillGLWindow() with 'Destroy = true' at SceneManager.cpp:1024.")
endif()

message(STATUS "AC-3 PASS: KillGLWindow() replaced with 'Destroy = true' in SceneManager.cpp")
