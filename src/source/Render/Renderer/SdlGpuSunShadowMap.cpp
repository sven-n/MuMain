#include "stdafx.h"

#include "Render/Renderer/SdlGpuSunShadowMap.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Renderer/SdlGpuPixelFormat.h"

namespace mu
{

Uint32 SdlGpuSunShadowMap::ResolutionFor(Uint32 requested)
{
    return requested < 1536u ? 1024u : requested < 3072u ? 2048u : 4096u;
}

bool SdlGpuSunShadowMap::CreateResources(SDL_GPUDevice* device)
{
    m_device = device;
    if (!m_sampler)
    {
        SDL_GPUSamplerCreateInfo samplerInfo{};
        samplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
        samplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
        samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
        samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        samplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        samplerInfo.enable_compare = true;
        // Lit where the surface is no further from the sun than what the map saw first.
        samplerInfo.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
        m_sampler = SDL_CreateGPUSampler(m_device, &samplerInfo);
    }
    if (!m_fallback)
    {
        m_fallback = CreateDepth(1u);
        m_fallbackCleared = false;
    }
    return IsAvailable();
}

bool SdlGpuSunShadowMap::IsAvailable() const
{
    return m_sampler && m_fallback;
}

void SdlGpuSunShadowMap::Release()
{
    ReleaseMap();
    if (m_fallback)
    {
        SDL_ReleaseGPUTexture(m_device, m_fallback);
        m_fallback = nullptr;
    }
    if (m_sampler)
    {
        SDL_ReleaseGPUSampler(m_device, m_sampler);
        m_sampler = nullptr;
    }
}

void SdlGpuSunShadowMap::Request(Uint32 requested)
{
    m_requested = true;
    m_resolution = ResolutionFor(requested);
}

void SdlGpuSunShadowMap::Cancel()
{
    m_requested = false;
}

SDL_GPUTexture* SdlGpuSunShadowMap::FallbackToClear() const
{
    return m_fallbackCleared ? nullptr : m_fallback;
}

SDL_GPUTexture* SdlGpuSunShadowMap::PrepareMap()
{
    m_ready = false;
    if (!m_requested)
    {
        ReleaseMap();
        return nullptr;
    }
    if (m_mapSize != m_resolution)
    {
        ReleaseMap();
        m_map = CreateDepth(m_resolution);
        m_mapSize = m_map ? m_resolution : 0u;
    }
    return m_map;
}

SDL_GPUTexture* SdlGpuSunShadowMap::CreateDepth(Uint32 size) const
{
    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SdlGpuDepthFormat;
    info.width = size;
    info.height = size;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    SDL_GPUTexture* const texture = SDL_CreateGPUTexture(m_device, &info);
    if (!texture)
    {
        mu::log::Get("render")->error("SDL_gpu -- sun shadow map creation failed ({0}x{0}): {1}", size,
                                      SDL_GetError());
    }
    return texture;
}

void SdlGpuSunShadowMap::ReleaseMap()
{
    if (m_map)
    {
        SDL_ReleaseGPUTexture(m_device, m_map);
        m_map = nullptr;
    }
    m_mapSize = 0u;
}

} // namespace mu
