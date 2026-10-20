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
    SDL_GPUBuffer* const bones[] = {Pointer<SDL_GPUBuffer>(4)};

    CHECK(state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(1)));
    CHECK(state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3)));
    CHECK(state.SelectVertexUniforms(0, uniforms));
    CHECK(state.SelectFragmentUniforms(uniforms));
    CHECK(state.SelectVertexStorageBuffers(bones));
    CHECK(state.SelectIndexBuffer(Pointer<SDL_GPUBuffer>(5), 16, SDL_GPU_INDEXELEMENTSIZE_16BIT));

    CHECK_FALSE(state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(1)));
    CHECK_FALSE(state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3)));
    CHECK_FALSE(state.SelectVertexUniforms(0, uniforms));
    CHECK_FALSE(state.SelectFragmentUniforms(uniforms));
    CHECK_FALSE(state.SelectVertexStorageBuffers(bones));
    CHECK_FALSE(state.SelectIndexBuffer(Pointer<SDL_GPUBuffer>(5), 16, SDL_GPU_INDEXELEMENTSIZE_16BIT));
}

TEST_CASE("pipeline changes invalidate descriptor-backed state [render][sdl_gpu_replay_state]")
{
    Render::SdlGpuReplayState state;
    const std::array<std::byte, 2> uniforms{std::byte{7}, std::byte{8}};
    SDL_GPUBuffer* const bones[] = {Pointer<SDL_GPUBuffer>(4)};

    state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(1));
    state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3));
    state.SelectVertexUniforms(0, uniforms);
    state.SelectFragmentUniforms(uniforms);
    state.SelectVertexStorageBuffers(bones);
    state.SelectIndexBuffer(Pointer<SDL_GPUBuffer>(5), 0, SDL_GPU_INDEXELEMENTSIZE_16BIT);

    CHECK(state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(6)));
    CHECK(state.SelectFragmentSampler(Pointer<SDL_GPUTexture>(2), Pointer<SDL_GPUSampler>(3)));
    CHECK(state.SelectVertexUniforms(0, uniforms));
    CHECK(state.SelectFragmentUniforms(uniforms));
    CHECK(state.SelectVertexStorageBuffers(bones));
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

TEST_CASE("the vertex buffer is bound once per vertex size [render][sdl_gpu_replay_state]")
{
    Render::SdlGpuReplayState state;

    CHECK(state.SelectVertexBuffer(Pointer<SDL_GPUBuffer>(1), 36, 72));
    CHECK(state.VertexBufferOffset() == 72);
    CHECK_FALSE(state.SelectVertexBuffer(Pointer<SDL_GPUBuffer>(1), 36, 360));
    state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(2));
    CHECK_FALSE(state.SelectVertexBuffer(Pointer<SDL_GPUBuffer>(1), 36, 396));
    CHECK(state.SelectVertexBuffer(Pointer<SDL_GPUBuffer>(3), 36, 396));

    state.Invalidate();
    CHECK(state.SelectVertexBuffer(Pointer<SDL_GPUBuffer>(3), 36, 396));
}

TEST_CASE("a new vertex size moves the binding to the draw's own offset [render][sdl_gpu_replay_state]")
{
    Render::SdlGpuReplayState state;

    CHECK(state.SelectVertexBuffer(Pointer<SDL_GPUBuffer>(1), 36, 0));
    CHECK(state.SelectVertexBuffer(Pointer<SDL_GPUBuffer>(1), 20, 400));
    CHECK(state.VertexBufferOffset() == 400);
    CHECK_FALSE(state.SelectVertexBuffer(Pointer<SDL_GPUBuffer>(1), 20, 480));
    // Before the binding: the draw cannot name a negative first vertex.
    CHECK(state.SelectVertexBuffer(Pointer<SDL_GPUBuffer>(1), 20, 200));
    CHECK(state.VertexBufferOffset() == 200);
}

TEST_CASE("the instance buffer is tracked apart from the vertex buffer [render][sdl_gpu_replay_state]")
{
    Render::SdlGpuReplayState state;

    CHECK(state.SelectVertexBuffer(Pointer<SDL_GPUBuffer>(1), 40, 0));
    CHECK(state.SelectInstanceBuffer(Pointer<SDL_GPUBuffer>(1)));
    CHECK(state.SelectVertexBuffer(Pointer<SDL_GPUBuffer>(2), 40, 0));
    CHECK_FALSE(state.SelectInstanceBuffer(Pointer<SDL_GPUBuffer>(1)));

    state.Invalidate();
    CHECK(state.SelectInstanceBuffer(Pointer<SDL_GPUBuffer>(1)));
}

TEST_CASE("a shadow draw's second uniform slot and storage buffer are tracked [render][sdl_gpu_replay_state]")
{
    Render::SdlGpuReplayState state;
    const std::array<std::byte, 2> skinning{std::byte{1}, std::byte{2}};
    const std::array<std::byte, 2> shadow{std::byte{3}, std::byte{4}};
    SDL_GPUBuffer* const bones[] = {Pointer<SDL_GPUBuffer>(5)};
    SDL_GPUBuffer* const bonesAndTerrain[] = {Pointer<SDL_GPUBuffer>(5), Pointer<SDL_GPUBuffer>(6)};

    state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(1));
    CHECK(state.SelectVertexUniforms(0, skinning));
    CHECK(state.SelectVertexUniforms(1, shadow));
    CHECK(state.SelectVertexStorageBuffers(bonesAndTerrain));
    CHECK_FALSE(state.SelectVertexUniforms(0, skinning));
    CHECK_FALSE(state.SelectVertexUniforms(1, shadow));
    CHECK_FALSE(state.SelectVertexStorageBuffers(bonesAndTerrain));

    // The slots are apart: the same bytes in the other slot are a change.
    CHECK(state.SelectVertexUniforms(1, skinning));
    CHECK_FALSE(state.SelectVertexUniforms(0, skinning));
    CHECK(state.SelectVertexStorageBuffers(bones));

    state.SelectPipeline(Pointer<SDL_GPUGraphicsPipeline>(2));
    CHECK(state.SelectVertexUniforms(1, skinning));
    CHECK(state.SelectVertexStorageBuffers(bones));
}
