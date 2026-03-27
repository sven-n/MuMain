# Story 7.9.1: AC-1 — Remove SwapBuffers dead calls from SceneManager.cpp and LoadingScene.cpp
# Flow Code: VS0-QUAL-RENDER-GAMELOOP
#
# RED PHASE:  Test FAILS before story 7.9.1 is implemented.
#             SwapBuffers(hDC) call present at SceneManager.cpp:968.
#             ::SwapBuffers(hDC) call present at LoadingScene.cpp:107.
#
# GREEN PHASE: Test PASSES after implementation.
#             Both SwapBuffers call sites are deleted. SDL3 EndFrame() handles presentation.
#             The shim in PlatformCompat.h remains as a safety net for any lingering call site.
#
# Validates:
#   AC-1 — SwapBuffers(hDC) removed from SceneManager.cpp (line 968)
#   AC-1 — ::SwapBuffers(hDC) removed from LoadingScene.cpp (line 107)
#   AC-STD-1 — No new Win32 API call sites in cross-platform render path

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SCENE_MANAGER_CPP)
    message(FATAL_ERROR "SCENE_MANAGER_CPP must be set to the path of SceneManager.cpp")
endif()

if(NOT DEFINED LOADING_SCENE_CPP)
    message(FATAL_ERROR "LOADING_SCENE_CPP must be set to the path of LoadingScene.cpp")
endif()

if(NOT EXISTS "${SCENE_MANAGER_CPP}")
    message(FATAL_ERROR "AC-1 FAIL: SceneManager.cpp not found at '${SCENE_MANAGER_CPP}'")
endif()

if(NOT EXISTS "${LOADING_SCENE_CPP}")
    message(FATAL_ERROR "AC-1 FAIL: LoadingScene.cpp not found at '${LOADING_SCENE_CPP}'")
endif()

file(READ "${SCENE_MANAGER_CPP}" scene_manager_content)
file(READ "${LOADING_SCENE_CPP}" loading_scene_content)

# ---------------------------------------------------------------------------
# AC-1a: SceneManager.cpp must not contain a bare SwapBuffers( call.
# The shim in PlatformCompat.h provides the type stub, but the call site must
# be removed entirely — not guarded, not commented out, simply gone.
# ---------------------------------------------------------------------------
set(found_violations FALSE)

string(FIND "${scene_manager_content}" "SwapBuffers(" sm_pos)
if(NOT sm_pos EQUAL -1)
    message(WARNING
        "AC-1 FAIL: 'SwapBuffers(' still present in SceneManager.cpp at position ${sm_pos}.\n"
        "Delete the SwapBuffers(hDC) call at SceneManager.cpp:968 — SDL3 EndFrame() handles presentation.")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-1b: LoadingScene.cpp must not contain a ::SwapBuffers( call.
# ---------------------------------------------------------------------------
string(FIND "${loading_scene_content}" "SwapBuffers(" ls_pos)
if(NOT ls_pos EQUAL -1)
    message(WARNING
        "AC-1 FAIL: 'SwapBuffers(' still present in LoadingScene.cpp at position ${ls_pos}.\n"
        "Delete the ::SwapBuffers(hDC) call at LoadingScene.cpp:107 — SDL3 EndFrame() handles presentation.")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-1 FAIL: SwapBuffers dead call(s) not yet removed.\n"
        "Story 7.9.1 Task 1: delete SwapBuffers(hDC) from SceneManager.cpp:968 and LoadingScene.cpp:107.\n"
        "Do NOT add #ifdef guards — migration strategy requires call site deletion, not guarding.")
endif()

message(STATUS "AC-1 PASS: SwapBuffers dead calls removed from SceneManager.cpp and LoadingScene.cpp")
