# Story 7.9.7: AC-3 — Perspective projection uses GLM with Z [0,1] depth convention
# Flow Code: VS0-RENDER-GLM-MATRIX
#
# GREEN PHASE: Test PASSES once GLM_FORCE_DEPTH_ZERO_TO_ONE is defined before
#              the glm:: perspective call, ensuring Metal/Vulkan Z [0,1] depth range.
#
# NOTE: AC-3 originally specified 'glm::perspectiveLH_ZO', but Task 2.11 removed
#       GLM_FORCE_LEFT_HANDED (game uses right-handed OpenGL convention).
#       The correct implementation is: GLM_FORCE_DEPTH_ZERO_TO_ONE + glm::perspective()
#       which equals perspectiveRH_ZO — correct for Metal/Vulkan.
#
# Validates:
#   AC-3 — GLM_FORCE_DEPTH_ZERO_TO_ONE defined in ZzzOpenglUtil.cpp (perspective path)
#   AC-3 — glm::perspective called (not hand-rolled trig)
#   AC-3 — GLM_FORCE_LEFT_HANDED absent (right-handed convention per Task 2.11)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED OPENGLUTIL_CPP)
    message(FATAL_ERROR "OPENGLUTIL_CPP must be set to the path of ZzzOpenglUtil.cpp")
endif()

if(NOT DEFINED RENDERER_CPP)
    message(FATAL_ERROR "RENDERER_CPP must be set to the path of MuRendererSDLGpu.cpp")
endif()

if(NOT EXISTS "${OPENGLUTIL_CPP}")
    message(FATAL_ERROR "AC-3 FAIL: ZzzOpenglUtil.cpp not found at '${OPENGLUTIL_CPP}'")
endif()

if(NOT EXISTS "${RENDERER_CPP}")
    message(FATAL_ERROR "AC-3 FAIL: MuRendererSDLGpu.cpp not found at '${RENDERER_CPP}'")
endif()

file(READ "${OPENGLUTIL_CPP}" openglutil_content)
file(READ "${RENDERER_CPP}" renderer_content)

set(found_violations FALSE)

# ---------------------------------------------------------------------------
# AC-3a: GLM_FORCE_DEPTH_ZERO_TO_ONE must be defined in ZzzOpenglUtil.cpp
# This ensures glm::perspective() uses Z [0,1] (Metal/Vulkan convention).
# ---------------------------------------------------------------------------
string(FIND "${openglutil_content}" "GLM_FORCE_DEPTH_ZERO_TO_ONE" depth_zero_util_pos)
if(depth_zero_util_pos EQUAL -1)
    message(WARNING
        "AC-3 FAIL: 'GLM_FORCE_DEPTH_ZERO_TO_ONE' not found in ZzzOpenglUtil.cpp.\n"
        "Story 7.9.7 Task 2.8: Add '#define GLM_FORCE_DEPTH_ZERO_TO_ONE' before glm includes in ZzzOpenglUtil.cpp.\n"
        "This sets Metal/Vulkan depth range Z [0,1] for the perspective matrix.")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-3b: GLM_FORCE_DEPTH_ZERO_TO_ONE must be defined in MuRendererSDLGpu.cpp
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "GLM_FORCE_DEPTH_ZERO_TO_ONE" depth_zero_renderer_pos)
if(depth_zero_renderer_pos EQUAL -1)
    message(WARNING
        "AC-3 FAIL: 'GLM_FORCE_DEPTH_ZERO_TO_ONE' not found in MuRendererSDLGpu.cpp.\n"
        "Story 7.9.7 Task 2.8: Add '#define GLM_FORCE_DEPTH_ZERO_TO_ONE' before glm includes in MuRendererSDLGpu.cpp.")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-3c: glm::perspective must be used (not hand-rolled trig)
# ---------------------------------------------------------------------------
string(FIND "${openglutil_content}" "glm::perspective" perspective_pos)
if(perspective_pos EQUAL -1)
    message(WARNING
        "AC-3 FAIL: 'glm::perspective' not found in ZzzOpenglUtil.cpp.\n"
        "Story 7.9.7 Task 2.8: Replace hand-rolled gluPerspective trig with glm::perspective().\n"
        "The local gluPerspective() function should call glm::perspective(glm::radians(fovy), aspect, zNear, zFar).")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-3d: GLM_FORCE_LEFT_HANDED must NOT be defined (Task 2.11 — right-handed)
# The game uses OpenGL's right-handed coordinate system.
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "GLM_FORCE_LEFT_HANDED" left_handed_pos)
if(NOT left_handed_pos EQUAL -1)
    message(WARNING
        "AC-3 FAIL: 'GLM_FORCE_LEFT_HANDED' found in MuRendererSDLGpu.cpp at position ${left_handed_pos}.\n"
        "Story 7.9.7 Task 2.11: Remove GLM_FORCE_LEFT_HANDED — game uses right-handed OpenGL convention.\n"
        "Use only GLM_FORCE_DEPTH_ZERO_TO_ONE for the depth range override.")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-3 FAIL: GLM depth convention not correctly configured.\n"
        "Story 7.9.7 Tasks 2.8 and 2.11: Use GLM_FORCE_DEPTH_ZERO_TO_ONE + glm::perspective() (right-handed).\n"
        "Remove GLM_FORCE_LEFT_HANDED if present.")
endif()

message(STATUS "AC-3 PASS: GLM_FORCE_DEPTH_ZERO_TO_ONE + glm::perspective (right-handed Z[0,1]) configured correctly")
