#include "SdlGpuEdgeSmoothing.h"

#include "Core/Utilities/Log/MuLogger.h"

namespace mu
{
namespace
{
// Three vertices cover the target with one triangle (post_fullscreen.vert.hlsl).
constexpr Uint32 FullscreenVertexCount = 3u;

SDL_GPUGraphicsPipeline* BuildPipeline(SDL_GPUDevice* device, const SdlGpuEdgeSmoothingShaders& shaders,
                                       SDL_GPUTextureFormat frameFormat, SDL_GPUSampleCount sampleCount)
{
    SDL_GPUColorTargetDescription colorTarget{};
    colorTarget.format = frameFormat;

    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = shaders.fullscreenVertex;
    info.fragment_shader = shaders.fxaa;
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    info.multisample_state.sample_count = sampleCount;
    info.target_info.color_target_descriptions = &colorTarget;
    info.target_info.num_color_targets = 1;
    info.target_info.has_depth_stencil_target = true;
    info.target_info.depth_stencil_format = SdlGpuDepthFormat;
    return SDL_CreateGPUGraphicsPipeline(device, &info);
}

SDL_GPUSampler* CreateLinearSampler(SDL_GPUDevice* device)
{
    SDL_GPUSamplerCreateInfo info{};
    info.min_filter = SDL_GPU_FILTER_LINEAR;
    info.mag_filter = SDL_GPU_FILTER_LINEAR;
    info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    return SDL_CreateGPUSampler(device, &info);
}
} // namespace

bool SdlGpuEdgeSmoothing::CreatePipelines(SDL_GPUDevice* device, const SdlGpuEdgeSmoothingShaders& shaders,
                                          SDL_GPUTextureFormat frameFormat, SDL_GPUSampleCount sampleCount)
{
    if (m_device && m_device != device)
    {
        // Handles of an earlier device, destroyed with it: forget them.
        *this = SdlGpuEdgeSmoothing{};
    }
    DestroyPipelines();
    if (!device || !shaders.fullscreenVertex || !shaders.fxaa)
    {
        return false;
    }
    m_device = device;
    if (frameFormat != m_frameFormat)
    {
        ReleaseImage();
    }
    m_frameFormat = frameFormat;
    m_pipeline = BuildPipeline(device, shaders, frameFormat, sampleCount);
    if (!m_linearSampler)
    {
        m_linearSampler = CreateLinearSampler(device);
    }
    if (!m_pipeline || !m_linearSampler)
    {
        log::Get("render")->error("SDL_gpu -- edge smoothing pipeline failed: {}", SDL_GetError());
        DestroyPipelines();
        return false;
    }
    return true;
}

void SdlGpuEdgeSmoothing::DestroyPipelines()
{
    if (m_pipeline)
    {
        SDL_ReleaseGPUGraphicsPipeline(m_device, m_pipeline);
        m_pipeline = nullptr;
    }
}

bool SdlGpuEdgeSmoothing::IsAvailable() const
{
    return m_pipeline && m_linearSampler;
}

SDL_GPUTexture* SdlGpuEdgeSmoothing::EnsureImage(SdlGpuTargetSize size)
{
    if (!IsAvailable() || size.width == 0u || size.height == 0u)
    {
        return nullptr;
    }
    if (m_image && m_size.width == size.width && m_size.height == size.height)
    {
        return m_image;
    }
    ReleaseImage();
    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = m_frameFormat;
    info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    info.width = size.width;
    info.height = size.height;
    info.layer_count_or_depth = 1u;
    info.num_levels = 1u;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    m_image = SDL_CreateGPUTexture(m_device, &info);
    if (!m_image)
    {
        log::Get("render")->error("SDL_gpu -- edge smoothing image failed ({}x{}): {}", size.width, size.height,
                                  SDL_GetError());
        return nullptr;
    }
    m_size = size;
    return m_image;
}

void SdlGpuEdgeSmoothing::Draw(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* pass, SdlGpuTargetSize size,
                               float subpixelStrength)
{
    if (!IsAvailable() || !m_image || size.width == 0u || size.height == 0u)
    {
        return;
    }
    const SDL_GPUViewport viewport{0.0f, 0.0f, static_cast<float>(size.width), static_cast<float>(size.height),
                                   0.0f, 1.0f};
    const SDL_Rect scissor{0, 0, static_cast<int>(size.width), static_cast<int>(size.height)};
    SDL_SetGPUViewport(pass, &viewport);
    SDL_SetGPUScissor(pass, &scissor);
    SDL_BindGPUGraphicsPipeline(pass, m_pipeline);
    const SDL_GPUTextureSamplerBinding binding{m_image, m_linearSampler};
    SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);
    const SdlGpuFxaaUniform uniform{
        {1.0f / static_cast<float>(m_size.width), 1.0f / static_cast<float>(m_size.height)}, subpixelStrength, 0.0f};
    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &uniform, sizeof(uniform));
    SDL_DrawGPUPrimitives(pass, FullscreenVertexCount, 1, 0, 0);
}

void SdlGpuEdgeSmoothing::ReleaseImage()
{
    if (m_image)
    {
        SDL_ReleaseGPUTexture(m_device, m_image);
        m_image = nullptr;
    }
    m_size = {};
}

void SdlGpuEdgeSmoothing::Release()
{
    DestroyPipelines();
    ReleaseImage();
    if (m_linearSampler)
    {
        SDL_ReleaseGPUSampler(m_device, m_linearSampler);
        m_linearSampler = nullptr;
    }
    m_device = nullptr;
    m_frameFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
}

} // namespace mu
