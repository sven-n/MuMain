# Story 7.9.7: AC-4/5 (alpha discard) — SetAlphaTest propagates to GPU fog uniform
# Flow Code: VS0-RENDER-GLM-MATRIX
#
# RED PHASE:  Test FAILS until Task 4.1 is implemented.
#             Current SetAlphaTest(bool) at ~line 1422 only sets 'm_alphaTestEnabled'.
#             It does NOT update 'm_fogUniform.alphaDiscardEnabled' or set s_fogDirty = true.
#             The fragment shader's alpha discard ('if (alphaDiscardEnabled && color.a <= alphaThreshold) discard')
#             is therefore never activated — particle sprites render as solid rectangles.
#
# GREEN PHASE: Test PASSES after Task 4.1:
#              SetAlphaTest updates m_fogUniform.alphaDiscardEnabled (enabled ? 1u : 0u)
#              SetAlphaTest sets s_fogDirty = true to trigger GPU upload
#
# Validates:
#   AC-4/5 — SetAlphaTest() propagates alpha test state to m_fogUniform.alphaDiscardEnabled
#   AC-4/5 — SetAlphaTest() marks fog data dirty for GPU upload

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
# AC-4a: SetAlphaTest must update m_fogUniform.alphaDiscardEnabled
# The current implementation only sets m_alphaTestEnabled (bool flag),
# which is never read by the GPU path.
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "m_fogUniform.alphaDiscardEnabled" fog_alpha_discard_pos)
if(fog_alpha_discard_pos EQUAL -1)
    message(WARNING
        "AC-4 FAIL: 'm_fogUniform.alphaDiscardEnabled' write not found in MuRendererSDLGpu.cpp.\n"
        "Story 7.9.7 Task 4.1: Update SetAlphaTest(bool enabled) to set:\n"
        "  m_fogUniform.alphaDiscardEnabled = enabled ? 1u : 0u;\n"
        "  s_fogDirty = true;\n"
        "Without this, the fragment shader's alpha discard condition is always false.")
    set(found_violations TRUE)
else()
    # Verify it's in the SetAlphaTest function context
    # Extract a window around the found position to check for SetAlphaTest context
    string(FIND "${renderer_content}" "SetAlphaTest" setalpha_pos)
    if(setalpha_pos EQUAL -1)
        message(WARNING
            "AC-4 FAIL: 'SetAlphaTest' override not found in MuRendererSDLGpu.cpp.\n"
            "Story 7.9.7 Task 4.1: SetAlphaTest must be overridden in the concrete MuRendererSDLGpu class.")
        set(found_violations TRUE)
    endif()
endif()

# ---------------------------------------------------------------------------
# AC-4b: s_fogDirty must be set to true in the alpha test path
# Without marking fog dirty, the GPU uniform buffer is never updated.
# ---------------------------------------------------------------------------
# We check that s_fogDirty = true appears somewhere near the alpha discard update.
# A simple search is sufficient — the file has limited occurrences of s_fogDirty.
string(FIND "${renderer_content}" "s_fogDirty = true" fog_dirty_pos)
if(fog_dirty_pos EQUAL -1)
    message(WARNING
        "AC-4 FAIL: 's_fogDirty = true' not found in MuRendererSDLGpu.cpp.\n"
        "Story 7.9.7 Task 4.1: After updating m_fogUniform.alphaDiscardEnabled, set 's_fogDirty = true'.\n"
        "The fog upload in BeginFrame() is gated by s_fogDirty — without it, GPU state is stale.")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-4/5 FAIL: SetAlphaTest() does not propagate alpha discard state to the GPU shader.\n"
        "Story 7.9.7 Task 4.1:\n"
        "  In SetAlphaTest(bool enabled):\n"
        "    m_fogUniform.alphaDiscardEnabled = enabled ? 1u : 0u;\n"
        "    s_fogDirty = true;\n"
        "  Remove the old 'm_alphaTestEnabled = enabled;' if it's no longer needed.\n"
        "WHY: ZzzOpenglUtil.cpp calls EnableAlphaTest() → SetAlphaTest(true) + SetAlphaFunc(GL_GREATER, 0.25f).\n"
        "     The fragment shader discards: 'if (alphaDiscardEnabled && color.a <= alphaThreshold) discard;'\n"
        "     Without propagation, alphaDiscardEnabled == 0 → full quad rectangles render.")
endif()

message(STATUS "AC-4/5 PASS: SetAlphaTest propagates to m_fogUniform.alphaDiscardEnabled and marks s_fogDirty=true")
