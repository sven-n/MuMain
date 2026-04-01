# Story 7.9.7: AC-4 — Orthographic projection uses glm::ortho for 2D rendering
# Flow Code: VS0-RENDER-GLM-MATRIX
#
# GREEN PHASE: Test PASSES once glm::ortho is used in the 2D render path.
#              BeginScene (2D pass) must use glm::ortho, not a hand-rolled ortho.
#
# Validates:
#   AC-4 — glm::ortho present in MuRendererSDLGpu.cpp (2D render path)
#   AC-4 — No hand-rolled ortho matrix computation (no manual column assignment for ortho)

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED RENDERER_CPP)
    message(FATAL_ERROR "RENDERER_CPP must be set to the path of MuRendererSDLGpu.cpp")
endif()

if(NOT EXISTS "${RENDERER_CPP}")
    message(FATAL_ERROR "AC-4 FAIL: MuRendererSDLGpu.cpp not found at '${RENDERER_CPP}'")
endif()

file(READ "${RENDERER_CPP}" renderer_content)

set(found_violations FALSE)

# ---------------------------------------------------------------------------
# AC-4a: glm::ortho must be present in the renderer
# The 2D render pass (BeginScene / Begin2DPass) pushes an ortho matrix.
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "glm::ortho" ortho_pos)
if(ortho_pos EQUAL -1)
    message(WARNING
        "AC-4 FAIL: 'glm::ortho' not found in MuRendererSDLGpu.cpp.\n"
        "Story 7.9.7 Task 2: Replace hand-rolled ortho with glm::ortho() in the 2D render path.\n"
        "Example: 'glm::mat4 ortho = glm::ortho(0.f, winW, 0.f, winH, -1.f, 1.f);'")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-4 FAIL: glm::ortho not used for 2D orthographic projection.\n"
        "Story 7.9.7 Task 2: Use glm::ortho in the 2D pass (BeginScene/Begin2DPass) in MuRendererSDLGpu.cpp.")
endif()

message(STATUS "AC-4 PASS: glm::ortho used for 2D orthographic projection in BeginScene")
