#pragma once

#include <array>
#include <cstddef>
#include <span>

#include <SDL3/SDL.h>

namespace Render
{
class SdlGpuReplayState final
{
public:
    bool SelectPipeline(SDL_GPUGraphicsPipeline* pipeline);
    bool SelectFragmentSampler(SDL_GPUTexture* texture, SDL_GPUSampler* sampler);
    bool SelectVertexUniforms(Uint32 slot, std::span<const std::byte> bytes);
    bool SelectFragmentUniforms(std::span<const std::byte> bytes);
    // The buffers bound from storage slot 0.
    bool SelectVertexStorageBuffers(std::span<SDL_GPUBuffer* const> buffers);
    // True when the buffer must be bound again, at VertexBufferOffset(), to draw `stride`-byte vertices
    // starting `offset` bytes in. A new stride always moves the binding: SDL's D3D12 backend takes the
    // stride from the pipeline only when the binding changes.
    bool SelectVertexBuffer(SDL_GPUBuffer* buffer, Uint32 stride, Uint32 offset);
    Uint32 VertexBufferOffset() const { return m_vertexBufferOffset; }
    // Slot 1, a skinned draw's per-instance colour, always bound at offset 0.
    bool SelectInstanceBuffer(SDL_GPUBuffer* buffer);
    bool SelectIndexBuffer(SDL_GPUBuffer* buffer, Uint32 offset, SDL_GPUIndexElementSize elementSize);
    bool SelectViewport(const SDL_GPUViewport& viewport);
    bool SelectScissor(const SDL_Rect& scissor);
    void Invalidate();

private:
    static constexpr std::size_t kVertexUniformSlots = 2;
    static constexpr std::size_t kVertexUniformCapacity = 192;
    static constexpr std::size_t kFragmentUniformCapacity = 48;
    static constexpr std::size_t kVertexStorageBufferCapacity = 2;

    std::array<std::array<std::byte, kVertexUniformCapacity>, kVertexUniformSlots> m_vertexUniforms{};
    std::array<std::byte, kFragmentUniformCapacity> m_fragmentUniforms{};
    std::array<std::size_t, kVertexUniformSlots> m_vertexUniformSize{};
    std::size_t m_fragmentUniformSize = 0;
    SDL_GPUGraphicsPipeline* m_pipeline = nullptr;
    SDL_GPUTexture* m_texture = nullptr;
    SDL_GPUSampler* m_sampler = nullptr;
    std::array<SDL_GPUBuffer*, kVertexStorageBufferCapacity> m_vertexStorageBuffers{};
    std::size_t m_vertexStorageBufferCount = 0;
    SDL_GPUBuffer* m_vertexBuffer = nullptr;
    Uint32 m_vertexStride = 0;
    Uint32 m_vertexBufferOffset = 0;
    SDL_GPUBuffer* m_instanceBuffer = nullptr;
    SDL_GPUBuffer* m_indexBuffer = nullptr;
    Uint32 m_indexOffset = 0;
    SDL_GPUIndexElementSize m_indexElementSize = SDL_GPU_INDEXELEMENTSIZE_16BIT;
    SDL_GPUViewport m_viewport{};
    SDL_Rect m_scissor{};
    bool m_pipelineValid = false;
    bool m_fragmentSamplerValid = false;
    std::array<bool, kVertexUniformSlots> m_vertexUniformsValid{};
    bool m_fragmentUniformsValid = false;
    bool m_vertexStorageBufferValid = false;
    bool m_vertexBufferValid = false;
    bool m_instanceBufferValid = false;
    bool m_indexBufferValid = false;
    bool m_viewportValid = false;
    bool m_scissorValid = false;
};
}
