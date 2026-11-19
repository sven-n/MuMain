#include <doctest.h>

#include "Render/Renderer/SdlGpuDepthState.h"

// Damage numbers and other DisableDepthTest() draws must pass every fragment on backends that read only
// the compare function (WebGPU), or they hide behind the models drawn before them.
TEST_CASE("a pipeline without the depth test passes every fragment and writes no depth")
{
    for (const bool depthWrite : {false, true})
    {
        const SDL_GPUDepthStencilState state = Render::MakeDepthStencilState(false, depthWrite);
        CHECK_FALSE(state.enable_depth_test);
        CHECK(state.compare_op == SDL_GPU_COMPAREOP_ALWAYS);
        CHECK_FALSE(state.enable_depth_write);
    }
}

TEST_CASE("a pipeline with the depth test compares less-or-equal and writes as asked")
{
    const SDL_GPUDepthStencilState writing = Render::MakeDepthStencilState(true, true);
    CHECK(writing.enable_depth_test);
    CHECK(writing.compare_op == SDL_GPU_COMPAREOP_LESS_OR_EQUAL);
    CHECK(writing.enable_depth_write);

    const SDL_GPUDepthStencilState readOnly = Render::MakeDepthStencilState(true, false);
    CHECK(readOnly.compare_op == SDL_GPU_COMPAREOP_LESS_OR_EQUAL);
    CHECK_FALSE(readOnly.enable_depth_write);
}
