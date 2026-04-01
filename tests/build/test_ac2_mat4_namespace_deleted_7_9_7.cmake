# Story 7.9.7: AC-2 — mat4:: namespace fully replaced with glm::
# Flow Code: VS0-RENDER-GLM-MATRIX
#
# GREEN PHASE: Test PASSES once the hand-rolled mat4:: namespace is deleted
#              from MuRendererSDLGpu.cpp and replaced with glm:: equivalents.
#
# Validates:
#   AC-2 — No 'namespace mat4' declaration in MuRendererSDLGpu.cpp
#   AC-2 — No 'mat4::' call sites (function calls from the old namespace)
#   AC-2 — glm::mat4 type used for matrix members

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED RENDERER_CPP)
    message(FATAL_ERROR "RENDERER_CPP must be set to the path of MuRendererSDLGpu.cpp")
endif()

if(NOT EXISTS "${RENDERER_CPP}")
    message(FATAL_ERROR "AC-2 FAIL: MuRendererSDLGpu.cpp not found at '${RENDERER_CPP}'")
endif()

file(READ "${RENDERER_CPP}" renderer_content)

set(found_violations FALSE)

# ---------------------------------------------------------------------------
# AC-2a: Hand-rolled mat4 namespace must not exist
# The old code had 'namespace mat4 {' with hand-rolled functions.
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "namespace mat4" ns_pos)
if(NOT ns_pos EQUAL -1)
    message(WARNING
        "AC-2 FAIL: 'namespace mat4' still present in MuRendererSDLGpu.cpp at position ${ns_pos}.\n"
        "Story 7.9.7 Task 2.9: Delete the entire 'namespace mat4 {}' block (~150 lines).\n"
        "Replace all mat4:: function calls with glm:: equivalents.")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-2b: No mat4:: call sites in the renderer
# Check for 'mat4::' call pattern (function calls from the deleted namespace)
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "mat4::" callsite_pos)
if(NOT callsite_pos EQUAL -1)
    message(WARNING
        "AC-2 FAIL: 'mat4::' call site still present in MuRendererSDLGpu.cpp at position ${callsite_pos}.\n"
        "Story 7.9.7 Task 2: Replace all mat4:: function calls with glm:: equivalents.\n"
        "See story Tasks 2.1-2.10 for the full list of replacements.")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-2c: glm::mat4 must be used (positive check — GLM is present)
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "glm::mat4" glm_mat4_pos)
if(glm_mat4_pos EQUAL -1)
    message(WARNING
        "AC-2 FAIL: 'glm::mat4' not found in MuRendererSDLGpu.cpp.\n"
        "Story 7.9.7 Task 2.10: Matrix stack members must use glm::mat4 type.\n"
        "Example: 'glm::mat4 m_projMatrix{{1.0f}};'")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-2 FAIL: Hand-rolled mat4:: math not fully replaced with glm::.\n"
        "Story 7.9.7 Tasks 2.1-2.10: Replace all mat4:: functions with GLM equivalents.\n"
        "Task 2.9: Delete 'namespace mat4 {}' block entirely.")
endif()

message(STATUS "AC-2 PASS: mat4:: namespace deleted; glm::mat4 in use")
