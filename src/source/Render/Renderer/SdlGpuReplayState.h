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
    bool SelectVertexUniforms(std::span<const std::byte> bytes);
    bool SelectFragmentUniforms(std::span<const std::byte> bytes);
    bool SelectVertexStorageBuffer(SDL_GPUBuffer* buffer);
    bool SelectIndexBuffer(SDL_GPUBuffer* buffer, Uint32 offset, SDL_GPUIndexElementSize elementSize);
    bool SelectViewport(const SDL_GPUViewport& viewport);
    bool SelectScissor(const SDL_Rect& scissor);
    void Invalidate();

private:
    static constexpr std::size_t kVertexUniformCapacity = 192;
    static constexpr std::size_t kFragmentUniformCapacity = 48;

    std::array<std::byte, kVertexUniformCapacity> m_vertexUniforms{};
    std::array<std::byte, kFragmentUniformCapacity> m_fragmentUniforms{};
    std::size_t m_vertexUniformSize = 0;
    std::size_t m_fragmentUniformSize = 0;
    SDL_GPUGraphicsPipeline* m_pipeline = nullptr;
    SDL_GPUTexture* m_texture = nullptr;
    SDL_GPUSampler* m_sampler = nullptr;
    SDL_GPUBuffer* m_vertexStorageBuffer = nullptr;
    SDL_GPUBuffer* m_indexBuffer = nullptr;
    Uint32 m_indexOffset = 0;
    SDL_GPUIndexElementSize m_indexElementSize = SDL_GPU_INDEXELEMENTSIZE_16BIT;
    SDL_GPUViewport m_viewport{};
    SDL_Rect m_scissor{};
    bool m_pipelineValid = false;
    bool m_fragmentSamplerValid = false;
    bool m_vertexUniformsValid = false;
    bool m_fragmentUniformsValid = false;
    bool m_vertexStorageBufferValid = false;
    bool m_indexBufferValid = false;
    bool m_viewportValid = false;
    bool m_scissorValid = false;
};
}
