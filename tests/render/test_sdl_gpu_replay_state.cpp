#include <array>
#include <cstddef>
#include <cstdint>

#include <doctest.h>

#include "Render/Renderer/SdlGpuReplayState.h"

namespace
{
template <typename T>
T* Pointer(std::uintptr_t value)
{
    return reinterpret_cast<T*>(value);
}
}

TEST_CASE("identical replay state is skipped [render][sdl_gpu_replay_state]")
{
    Render::SdlGpuReplayState state;
    const std::array<std::byte, 4> uniforms{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};

    CHECK(state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(1)));
    CHECK(state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3)));
    CHECK(state.SelectVertexUniforms(uniforms));
    CHECK(state.SelectFragmentUniforms(uniforms));
    CHECK(state.SelectVertexStorageBuffer(Pointer<SDL_GPUBuffer>(4)));
    CHECK(state.SelectIndexBuffer(Pointer<SDL_GPUBuffer>(5), 16, SDL_GPU_INDEXELEMENTSIZE_16BIT));

    CHECK_FALSE(state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(1)));
    CHECK_FALSE(state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3)));
    CHECK_FALSE(state.SelectVertexUniforms(uniforms));
    CHECK_FALSE(state.SelectFragmentUniforms(uniforms));
    CHECK_FALSE(state.SelectVertexStorageBuffer(Pointer<SDL_GPUBuffer>(4)));
    CHECK_FALSE(state.SelectIndexBuffer(Pointer<SDL_GPUBuffer>(5), 16, SDL_GPU_INDEXELEMENTSIZE_16BIT));
}

TEST_CASE("pipeline changes invalidate descriptor-backed state [render][sdl_gpu_replay_state]")
{
    Render::SdlGpuReplayState state;
    const std::array<std::byte, 2> uniforms{std::byte{7}, std::byte{8}};

    state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(1));
    state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3));
    state.SelectVertexUniforms(uniforms);
    state.SelectFragmentUniforms(uniforms);
    state.SelectVertexStorageBuffer(Pointer<SDL_GPUBuffer>(4));
    state.SelectIndexBuffer(Pointer<SDL_GPUBuffer>(5), 0, SDL_GPU_INDEXELEMENTSIZE_16BIT);

    CHECK(state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(6)));
    CHECK(state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3)));
    CHECK(state.SelectVertexUniforms(uniforms));
    CHECK(state.SelectFragmentUniforms(uniforms));
    CHECK(state.SelectVertexStorageBuffer(Pointer<SDL_GPUBuffer>(4)));
    CHECK_FALSE(state.SelectIndexBuffer(Pointer<SDL_GPUBuffer>(5), 0, SDL_GPU_INDEXELEMENTSIZE_16BIT));
}

TEST_CASE("viewport scissor and editor invalidation are explicit [render][sdl_gpu_replay_state]")
{
    Render::SdlGpuReplayState state;
    const SDL_GPUViewport viewport{1.0f, 2.0f, 640.0f, 480.0f, 0.0f, 1.0f};
    const SDL_Rect scissor{1, 2, 640, 480};

    CHECK(state.SelectViewport(viewport));
    CHECK(state.SelectScissor(scissor));
    CHECK_FALSE(state.SelectViewport(viewport));
    CHECK_FALSE(state.SelectScissor(scissor));

    state.Invalidate();
    CHECK(state.SelectViewport(viewport));
    CHECK(state.SelectScissor(scissor));
}
