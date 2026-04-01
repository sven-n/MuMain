# Story 7.9.7: AC-7 — SetAlphaFunc override updates fog uniform alpha threshold
# Flow Code: VS0-RENDER-GLM-MATRIX
#
# RED PHASE:  Test FAILS until Task 4.2 is implemented.
#             SetAlphaFunc(int, float) is inherited as a no-op from IMuRenderer base class.
#             There is no SetAlphaFunc override in the concrete MuRendererSDLGpu class.
#             Therefore 'm_fogUniform.alphaThreshold' is never set from game code.
#             The fragment shader threshold defaults to 0.0f, discarding nothing or everything.
#
# GREEN PHASE: Test PASSES after Task 4.2:
#              SetAlphaFunc override added to MuRendererSDLGpu concrete class
#              Override updates m_fogUniform.alphaThreshold = ref
#              Override sets s_fogDirty = true
#
# Validates:
#   AC-7 — SetAlphaFunc(int func, float ref) override exists in MuRendererSDLGpu.cpp
#   AC-7 — Override updates m_fogUniform.alphaThreshold
#   AC-7 — Override sets s_fogDirty = true

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED RENDERER_CPP)
    message(FATAL_ERROR "RENDERER_CPP must be set to the path of MuRendererSDLGpu.cpp")
endif()

if(NOT EXISTS "${RENDERER_CPP}")
    message(FATAL_ERROR "AC-7 FAIL: MuRendererSDLGpu.cpp not found at '${RENDERER_CPP}'")
endif()

file(READ "${RENDERER_CPP}" renderer_content)

set(found_violations FALSE)

# ---------------------------------------------------------------------------
# AC-7a: SetAlphaFunc override must exist in the concrete renderer
# The base IMuRenderer::SetAlphaFunc is a no-op. The concrete class must override it.
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "SetAlphaFunc" setalphafunc_pos)
if(setalphafunc_pos EQUAL -1)
    message(WARNING
        "AC-7 FAIL: 'SetAlphaFunc' not found in MuRendererSDLGpu.cpp.\n"
        "Story 7.9.7 Task 4.2: Add SetAlphaFunc(int func, float ref) override to MuRendererSDLGpu concrete class.\n"
        "The base class has a no-op default — this override is required to route the GL_GREATER threshold to the shader.")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-7b: SetAlphaFunc must update m_fogUniform.alphaThreshold
# The threshold is the GL 'ref' parameter from SetAlphaFunc(GL_GREATER, 0.25f).
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "m_fogUniform.alphaThreshold" fog_threshold_pos)
if(fog_threshold_pos EQUAL -1)
    message(WARNING
        "AC-7 FAIL: 'm_fogUniform.alphaThreshold' write not found in MuRendererSDLGpu.cpp.\n"
        "Story 7.9.7 Task 4.2: In SetAlphaFunc(int func, float ref):\n"
        "  m_fogUniform.alphaThreshold = ref;\n"
        "  s_fogDirty = true;\n"
        "WITHOUT this: game calls SetAlphaFunc(GL_GREATER, 0.25f) → threshold stays 0.0f in shader.")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-7 FAIL: SetAlphaFunc override missing or does not update GPU fog uniform.\n"
        "Story 7.9.7 Task 4.2: Add to MuRendererSDLGpu concrete class:\n"
        "  void SetAlphaFunc(int /*func*/, float ref) override\n"
        "  {\n"
        "      m_fogUniform.alphaThreshold = ref;\n"
        "      s_fogDirty = true;\n"
        "  }\n"
        "WHY: ZzzOpenglUtil.cpp EnableAlphaTest() calls SetAlphaFunc(GL_GREATER, 0.25f).\n"
        "     Fragment shader: 'if (alphaDiscardEnabled && color.a <= alphaThreshold) discard;'\n"
        "     Without the threshold, alphaThreshold == 0.0f → discard fires for all transparent pixels\n"
        "     OR (with alphaDiscardEnabled=0) nothing is discarded. Either way the sprite is wrong.")
endif()

message(STATUS "AC-7 PASS: SetAlphaFunc override exists and updates m_fogUniform.alphaThreshold + s_fogDirty")
