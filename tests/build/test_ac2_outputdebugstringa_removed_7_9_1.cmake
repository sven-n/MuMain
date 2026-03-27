# Story 7.9.1: AC-2 — Replace OutputDebugStringA with g_ErrorReport.Write() in SceneManager.cpp
# Flow Code: VS0-QUAL-RENDER-GAMELOOP
#
# RED PHASE:  Test FAILS before story 7.9.1 is implemented.
#             OutputDebugStringA(errorMsg) calls are present at SceneManager.cpp:993
#             (MainScene catch block) and SceneManager.cpp:1032 (RenderScene catch block).
#             The PlatformCompat.h shim maps these to no-ops on macOS, silently
#             discarding exception information.
#
# GREEN PHASE: Test PASSES after implementation.
#             Both OutputDebugStringA calls are replaced with g_ErrorReport.Write().
#             The char errorMsg[256] / sprintf_s intermediary buffers are also removed.
#
# Validates:
#   AC-2 — OutputDebugStringA(errorMsg) replaced at SceneManager.cpp:993
#   AC-2 — OutputDebugStringA(errorMsg) replaced at SceneManager.cpp:1032
#   AC-STD-1 — Exceptions logged to g_ErrorReport, not silently discarded

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SCENE_MANAGER_CPP)
    message(FATAL_ERROR "SCENE_MANAGER_CPP must be set to the path of SceneManager.cpp")
endif()

if(NOT EXISTS "${SCENE_MANAGER_CPP}")
    message(FATAL_ERROR "AC-2 FAIL: SceneManager.cpp not found at '${SCENE_MANAGER_CPP}'")
endif()

file(READ "${SCENE_MANAGER_CPP}" scene_manager_content)

# ---------------------------------------------------------------------------
# AC-2: Check that OutputDebugStringA is not called in the cross-platform section.
# Strategy: split on the first "#ifdef _WIN32" — the pre-guard portion must be free
# of OutputDebugStringA calls. If no guard exists, the whole file is tested.
# ---------------------------------------------------------------------------
string(FIND "${scene_manager_content}" "#ifdef _WIN32" guard_pos)
if(guard_pos EQUAL -1)
    set(cross_platform_section "${scene_manager_content}")
else()
    string(SUBSTRING "${scene_manager_content}" 0 ${guard_pos} cross_platform_section)
endif()

set(found_violations FALSE)

string(FIND "${cross_platform_section}" "OutputDebugStringA(" ods_pos)
if(NOT ods_pos EQUAL -1)
    message(WARNING
        "AC-2 FAIL: 'OutputDebugStringA(' still present in SceneManager.cpp cross-platform section "
        "(position ${ods_pos}).\n"
        "Replace with g_ErrorReport.Write(L\"Exception in MainScene: %S\\r\\n\", e.what()) at :993\n"
        "Replace with g_ErrorReport.Write(L\"Exception in RenderScene: %S\\r\\n\", e.what()) at :1032\n"
        "Also remove char errorMsg[256] / sprintf_s intermediary buffers in both catch blocks.")
    set(found_violations TRUE)
endif()

# Also verify g_ErrorReport.Write is now present in SceneManager.cpp (confirms migration done)
string(FIND "${scene_manager_content}" "g_ErrorReport.Write" ger_pos)
if(ger_pos EQUAL -1)
    message(WARNING
        "AC-2 FAIL: 'g_ErrorReport.Write' not found in SceneManager.cpp.\n"
        "Add g_ErrorReport.Write() calls in the MainScene and RenderScene catch blocks.")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-2 FAIL: OutputDebugStringA calls not yet replaced with g_ErrorReport.Write().\n"
        "Story 7.9.1 Task 2: replace OutputDebugStringA in SceneManager.cpp:993 and :1032.")
endif()

message(STATUS "AC-2 PASS: OutputDebugStringA replaced with g_ErrorReport.Write() in SceneManager.cpp")
