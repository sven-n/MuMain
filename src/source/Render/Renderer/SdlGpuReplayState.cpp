#include "SdlGpuReplayState.h"

#include <algorithm>

namespace
{
template <std::size_t Capacity>
bool SelectBytes(std::span<const std::byte> bytes, std::array<std::byte, Capacity>& selectedBytes,
                 std::size_t& selectedSize, bool& valid)
{
    if (valid && selectedSize == bytes.size() && std::equal(bytes.begin(), bytes.end(), selectedBytes.begin()))
        return false;

    if (bytes.size() > selectedBytes.size())
    {
        valid = false;
        selectedSize = 0;
        return true;
    }

    std::copy(bytes.begin(), bytes.end(), selectedBytes.begin());
    selectedSize = bytes.size();
    valid = true;
    return true;
}
}

namespace Render
{
bool SdlGpuReplayState::SelectPipeline(SDL_GPUGraphicsPipeline* pipeline)
{
    if (m_pipelineValid && m_pipeline == pipeline)
        return false;

    m_pipeline = pipeline;
    m_pipelineValid = true;
    m_fragmentSamplerValid = false;
    m_vertexUniformsValid.fill(false);
    m_fragmentUniformsValid = false;
    m_vertexStorageBufferValid = false;
    return true;
}

bool SdlGpuReplayState::SelectFragmentSampler(SDL_GPUTexture* texture, SDL_GPUSampler* sampler)
{
    if (m_fragmentSamplerValid && m_texture == texture && m_sampler == sampler)
        return false;

    m_texture = texture;
    m_sampler = sampler;
    m_fragmentSamplerValid = true;
    return true;
}

bool SdlGpuReplayState::SelectVertexUniforms(Uint32 slot, std::span<const std::byte> bytes)
{
    if (slot >= kVertexUniformSlots)
        return true;

    bool valid = m_vertexUniformsValid[slot];
    const bool changed = SelectBytes(bytes, m_vertexUniforms[slot], m_vertexUniformSize[slot], valid);
    m_vertexUniformsValid[slot] = valid;
    return changed;
}

bool SdlGpuReplayState::SelectFragmentUniforms(std::span<const std::byte> bytes)
{
    return SelectBytes(bytes, m_fragmentUniforms, m_fragmentUniformSize, m_fragmentUniformsValid);
}

bool SdlGpuReplayState::SelectVertexStorageBuffers(std::span<SDL_GPUBuffer* const> buffers)
{
    if (m_vertexStorageBufferValid && m_vertexStorageBufferCount == buffers.size() &&
        std::equal(buffers.begin(), buffers.end(), m_vertexStorageBuffers.begin()))
    {
        return false;
    }

    m_vertexStorageBufferValid = buffers.size() <= m_vertexStorageBuffers.size();
    m_vertexStorageBufferCount = m_vertexStorageBufferValid ? buffers.size() : 0;
    std::copy_n(buffers.begin(), m_vertexStorageBufferCount, m_vertexStorageBuffers.begin());
    return true;
}

bool SdlGpuReplayState::SelectVertexBuffer(SDL_GPUBuffer* buffer, Uint32 stride, Uint32 offset)
{
    if (m_vertexBufferValid && m_vertexBuffer == buffer && m_vertexStride == stride && offset >= m_vertexBufferOffset &&
        (offset - m_vertexBufferOffset) % stride == 0)
    {
        return false;
    }

    m_vertexBuffer = buffer;
    m_vertexStride = stride;
    m_vertexBufferOffset = offset;
    m_vertexBufferValid = true;
    return true;
}

bool SdlGpuReplayState::SelectInstanceBuffer(SDL_GPUBuffer* buffer)
{
    if (m_instanceBufferValid && m_instanceBuffer == buffer)
        return false;

    m_instanceBuffer = buffer;
    m_instanceBufferValid = true;
    return true;
}

bool SdlGpuReplayState::SelectIndexBuffer(SDL_GPUBuffer* buffer, Uint32 offset, SDL_GPUIndexElementSize elementSize)
{
    if (m_indexBufferValid && m_indexBuffer == buffer && m_indexOffset == offset &&
        m_indexElementSize == elementSize)
    {
        return false;
    }

    m_indexBuffer = buffer;
    m_indexOffset = offset;
    m_indexElementSize = elementSize;
    m_indexBufferValid = true;
    return true;
}

bool SdlGpuReplayState::SelectViewport(const SDL_GPUViewport& viewport)
{
    if (m_viewportValid && m_viewport.x == viewport.x && m_viewport.y == viewport.y &&
        m_viewport.w == viewport.w && m_viewport.h == viewport.h &&
        m_viewport.min_depth == viewport.min_depth && m_viewport.max_depth == viewport.max_depth)
    {
        return false;
    }

    m_viewport = viewport;
    m_viewportValid = true;
    return true;
}

bool SdlGpuReplayState::SelectScissor(const SDL_Rect& scissor)
{
    if (m_scissorValid && m_scissor.x == scissor.x && m_scissor.y == scissor.y && m_scissor.w == scissor.w &&
        m_scissor.h == scissor.h)
    {
        return false;
    }

    m_scissor = scissor;
    m_scissorValid = true;
    return true;
}

void SdlGpuReplayState::Invalidate()
{
    m_pipelineValid = false;
    m_fragmentSamplerValid = false;
    m_vertexUniformsValid.fill(false);
    m_fragmentUniformsValid = false;
    m_vertexStorageBufferValid = false;
    m_vertexBufferValid = false;
    m_instanceBufferValid = false;
    m_indexBufferValid = false;
    m_viewportValid = false;
    m_scissorValid = false;
}
}
