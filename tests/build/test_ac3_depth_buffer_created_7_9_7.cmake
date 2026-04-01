# Story 7.9.7: AC-3 (depth buffer) — Depth buffer created and attached to render pass
# Flow Code: VS0-RENDER-GLM-MATRIX
#
# RED PHASE:  Test FAILS until Task 3 is implemented.
#             'has_depth_stencil_target = false' still present at ~line 1867.
#             No depth texture creation code (SDL_GPU_TEXTUREFORMAT_D24_UNORM) in Init().
#             SDL_BeginGPURenderPass depth target is nullptr.
#
# GREEN PHASE: Test PASSES after Task 3:
#              has_depth_stencil_target = true in pipeline creation
#              SDL_GPU_TEXTUREFORMAT_D24_UNORM (or D32_FLOAT) in Init()
#              Depth target passed to SDL_BeginGPURenderPass (not nullptr)
#
# Validates:
#   AC-3 (depth buffer) — Depth texture created at swapchain dimensions in Init()
#   AC-3 (depth buffer) — Depth target attached to render pass (not nullptr)
#   AC-3 (depth buffer) — Pipeline has_depth_stencil_target = true

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED RENDERER_CPP)
    message(FATAL_ERROR "RENDERER_CPP must be set to the path of MuRendererSDLGpu.cpp")
endif()

if(NOT EXISTS "${RENDERER_CPP}")
    message(FATAL_ERROR "AC-3 FAIL: MuRendererSDLGpu.cpp not found at '${RENDERER_CPP}'")
endif()

file(READ "${RENDERER_CPP}" renderer_content)

set(found_violations FALSE)

# ---------------------------------------------------------------------------
# AC-3 depth-a: has_depth_stencil_target must be TRUE in pipeline creation
# The old code sets 'has_depth_stencil_target = false' — this must become true.
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "has_depth_stencil_target = false" depth_false_pos)
if(NOT depth_false_pos EQUAL -1)
    message(WARNING
        "AC-3 FAIL: 'has_depth_stencil_target = false' still present in MuRendererSDLGpu.cpp at position ${depth_false_pos}.\n"
        "Story 7.9.7 Task 3.3: Change 'has_depth_stencil_target = false' to 'has_depth_stencil_target = true'.\n"
        "File: MuRendererSDLGpu.cpp, BuildBlendPipeline() ~line 1867.")
    set(found_violations TRUE)
endif()

# Verify the fix is present (positive check)
string(FIND "${renderer_content}" "has_depth_stencil_target = true" depth_true_pos)
if(depth_true_pos EQUAL -1)
    message(WARNING
        "AC-3 FAIL: 'has_depth_stencil_target = true' not found in MuRendererSDLGpu.cpp.\n"
        "Story 7.9.7 Task 3.3: Set 'has_depth_stencil_target = true' in BuildBlendPipeline() ~line 1867.")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-3 depth-b: Depth texture must be created with a depth format
# Check for D24_UNORM or D32_FLOAT depth format (either is acceptable)
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "SDL_GPU_TEXTUREFORMAT_D24_UNORM" d24_pos)
string(FIND "${renderer_content}" "SDL_GPU_TEXTUREFORMAT_D32_FLOAT" d32_pos)

if(d24_pos EQUAL -1 AND d32_pos EQUAL -1)
    message(WARNING
        "AC-3 FAIL: No depth texture format (SDL_GPU_TEXTUREFORMAT_D24_UNORM or D32_FLOAT) found in MuRendererSDLGpu.cpp.\n"
        "Story 7.9.7 Task 3.1: Create SDL_GPUTexture with depth format in Init().\n"
        "Use SDL_GPU_TEXTUREFORMAT_D24_UNORM (preferred) or SDL_GPU_TEXTUREFORMAT_D32_FLOAT.\n"
        "Texture dimensions must match swapchain dimensions.")
    set(found_violations TRUE)
endif()

# ---------------------------------------------------------------------------
# AC-3 depth-c: SDL_GPUDepthStencilTargetInfo must be used for depth attachment
# The old code passes nullptr as the depth target — must pass a valid struct.
# ---------------------------------------------------------------------------
string(FIND "${renderer_content}" "SDL_GPUDepthStencilTargetInfo" depth_target_info_pos)
if(depth_target_info_pos EQUAL -1)
    message(WARNING
        "AC-3 FAIL: 'SDL_GPUDepthStencilTargetInfo' not found in MuRendererSDLGpu.cpp.\n"
        "Story 7.9.7 Task 3.2: Pass SDL_GPUDepthStencilTargetInfo as depth target to SDL_BeginGPURenderPass().\n"
        "The current call at ~line 726 passes nullptr — this must be replaced with a valid depth struct.")
    set(found_violations TRUE)
endif()

if(found_violations)
    message(FATAL_ERROR
        "AC-3 FAIL: Depth buffer not created or not attached to render pass.\n"
        "Story 7.9.7 Task 3:\n"
        "  3.1: Create SDL_GPUTexture with D24_UNORM depth format in Init()\n"
        "  3.2: Pass SDL_GPUDepthStencilTargetInfo to SDL_BeginGPURenderPass() (not nullptr)\n"
        "  3.3: Set has_depth_stencil_target = true in BuildBlendPipeline() ~line 1867\n"
        "  3.4: Set depth clear to 1.0 with SDL_GPU_LOADOP_CLEAR\n"
        "  3.5: Recreate depth texture on window resize")
endif()

message(STATUS "AC-3 PASS: Depth buffer created (D24/D32 format) and attached to render pass with has_depth_stencil_target=true")
