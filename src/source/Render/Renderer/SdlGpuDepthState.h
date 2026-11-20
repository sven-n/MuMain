#pragma once

#include <SDL3/SDL_gpu.h>

namespace Render
{

// LESS_OR_EQUAL, as the classic client's glDepthFunc(GL_LEQUAL): models drawn in several passes redraw at equal depth.
// With the test off, ALWAYS and no write: WebGPU ignores enable_depth_test and reads only these two.
[[nodiscard]] inline SDL_GPUDepthStencilState MakeDepthStencilState(bool depthTest, bool depthWrite)
{
    SDL_GPUDepthStencilState depthState{};
    depthState.enable_depth_test = depthTest;
    depthState.enable_depth_write = depthTest && depthWrite;
    depthState.enable_stencil_test = false;
    depthState.compare_op = depthTest ? SDL_GPU_COMPAREOP_LESS_OR_EQUAL : SDL_GPU_COMPAREOP_ALWAYS;
    return depthState;
}

} // namespace Render
