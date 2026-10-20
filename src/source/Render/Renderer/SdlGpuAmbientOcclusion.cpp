#include "SdlGpuAmbientOcclusion.h"

#include "Core/Utilities/Log/MuLogger.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cstring>

namespace mu
{
namespace
{
// Three vertices cover the target with one triangle (post_fullscreen.vert.hlsl).
constexpr Uint32 FullscreenVertexCount = 3u;
constexpr SDL_GPUTextureFormat OcclusionFormat = SDL_GPU_TEXTUREFORMAT_R8_UNORM;

SDL_GPUGraphicsPipeline* BuildPipeline(SDL_GPUDevice* device, SDL_GPUShader* vertexShader,
                                       SDL_GPUShader* fragmentShader)
{
    SDL_GPUColorTargetDescription colorTarget{};
    colorTarget.format = OcclusionFormat;

    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vertexShader;
    info.fragment_shader = fragmentShader;
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    info.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.target_info.color_target_descriptions = &colorTarget;
    info.target_info.num_color_targets = 1;
    return SDL_CreateGPUGraphicsPipeline(device, &info);
}

SDL_GPUTexture* CreateTarget(SDL_GPUDevice* device, SdlGpuTargetSize size)
{
    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = OcclusionFormat;
    info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    info.width = size.width;
    info.height = size.height;
    info.layer_count_or_depth = 1u;
    info.num_levels = 1u;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    return SDL_CreateGPUTexture(device, &info);
}

void ReleaseTexture(SDL_GPUDevice* device, SDL_GPUTexture*& texture)
{
    if (texture)
    {
        SDL_ReleaseGPUTexture(device, texture);
        texture = nullptr;
    }
}

void ReleasePipeline(SDL_GPUDevice* device, SDL_GPUGraphicsPipeline*& pipeline)
{
    if (pipeline)
    {
        SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
        pipeline = nullptr;
    }
}
} // namespace

bool SdlGpuAmbientOcclusion::CreatePipelines(SDL_GPUDevice* device, const SdlGpuAmbientOcclusionShaders& shaders)
{
    if (m_device && m_device != device)
    {
        // Handles of an earlier device, destroyed with it: forget them.
        *this = SdlGpuAmbientOcclusion{};
    }
    DestroyPipelines();
    if (!device || !shaders.fullscreenVertex || !shaders.occlusion || !shaders.blur)
    {
        return false;
    }
    m_device = device;
    m_occlusionPipeline = BuildPipeline(device, shaders.fullscreenVertex, shaders.occlusion);
    m_blurPipeline = BuildPipeline(device, shaders.fullscreenVertex, shaders.blur);
    if (!m_pointSampler)
    {
        SDL_GPUSamplerCreateInfo info{};
        info.min_filter = SDL_GPU_FILTER_NEAREST;
        info.mag_filter = SDL_GPU_FILTER_NEAREST;
        info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
        info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        m_pointSampler = SDL_CreateGPUSampler(device, &info);
    }
    if (!IsAvailable())
    {
        log::Get("render")->warn("SDL_gpu -- ambient occlusion unavailable: {}", SDL_GetError());
        DestroyPipelines();
        return false;
    }
    return true;
}

void SdlGpuAmbientOcclusion::DestroyPipelines()
{
    ReleasePipeline(m_device, m_occlusionPipeline);
    ReleasePipeline(m_device, m_blurPipeline);
}

bool SdlGpuAmbientOcclusion::IsAvailable() const
{
    return m_occlusionPipeline && m_blurPipeline && m_pointSampler;
}

SDL_GPUTexture* SdlGpuAmbientOcclusion::Render(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture* depth,
                                               SdlGpuTargetSize size, const float (&projection)[16], float radius)
{
    if (!IsAvailable() || !depth || !EnsureTargets(size))
    {
        return nullptr;
    }

    SdlGpuAmbientOcclusionUniform uniform{};
    const glm::mat4 projectionMatrix = glm::make_mat4(projection);
    std::memcpy(uniform.projection, projection, sizeof(uniform.projection));
    std::memcpy(uniform.inverseProjection, glm::value_ptr(glm::inverse(projectionMatrix)),
                sizeof(uniform.inverseProjection));
    uniform.depthTexelSize[0] = 1.0f / static_cast<float>(size.width);
    uniform.depthTexelSize[1] = 1.0f / static_cast<float>(size.height);
    uniform.radius = radius;

    const SDL_GPUTextureSamplerBinding occlusionInputs[] = {{depth, m_pointSampler}};
    if (!RunPass(commandBuffer, m_occlusionPipeline, m_occlusion, occlusionInputs, 1u, uniform))
    {
        return nullptr;
    }
    const SDL_GPUTextureSamplerBinding blurInputs[] = {{depth, m_pointSampler}, {m_occlusion, m_pointSampler}};
    return RunPass(commandBuffer, m_blurPipeline, m_blurred, blurInputs, 2u, uniform) ? m_blurred : nullptr;
}

void SdlGpuAmbientOcclusion::Release()
{
    DestroyPipelines();
    ReleaseTargets();
    if (m_pointSampler)
    {
        SDL_ReleaseGPUSampler(m_device, m_pointSampler);
        m_pointSampler = nullptr;
    }
    m_device = nullptr;
}

bool SdlGpuAmbientOcclusion::EnsureTargets(SdlGpuTargetSize size)
{
    const SdlGpuTargetSize half{std::max(size.width / 2u, 1u), std::max(size.height / 2u, 1u)};
    if (m_occlusion && m_size.width == half.width && m_size.height == half.height)
    {
        return true;
    }
    ReleaseTargets();
    m_occlusion = CreateTarget(m_device, half);
    m_blurred = CreateTarget(m_device, half);
    if (!m_occlusion || !m_blurred)
    {
        log::Get("render")->error("SDL_gpu -- ambient occlusion targets failed ({}x{}): {}", half.width, half.height,
                                  SDL_GetError());
        ReleaseTargets();
        return false;
    }
    m_size = half;
    return true;
}

void SdlGpuAmbientOcclusion::ReleaseTargets()
{
    ReleaseTexture(m_device, m_occlusion);
    ReleaseTexture(m_device, m_blurred);
    m_size = {};
}

bool SdlGpuAmbientOcclusion::RunPass(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUGraphicsPipeline* pipeline,
                                     SDL_GPUTexture* target, const SDL_GPUTextureSamplerBinding* inputs,
                                     Uint32 inputCount, const SdlGpuAmbientOcclusionUniform& uniform)
{
    SDL_GPUColorTargetInfo colorTarget{};
    colorTarget.texture = target;
    colorTarget.load_op = SDL_GPU_LOADOP_DONT_CARE;
    colorTarget.store_op = SDL_GPU_STOREOP_STORE;
    colorTarget.cycle = true;
    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(commandBuffer, &colorTarget, 1, nullptr);
    if (!pass)
    {
        return false;
    }
    const SDL_GPUViewport viewport{0.0f, 0.0f, static_cast<float>(m_size.width), static_cast<float>(m_size.height),
                                   0.0f, 1.0f};
    const SDL_Rect scissor{0, 0, static_cast<int>(m_size.width), static_cast<int>(m_size.height)};
    SDL_SetGPUViewport(pass, &viewport);
    SDL_SetGPUScissor(pass, &scissor);
    SDL_BindGPUGraphicsPipeline(pass, pipeline);
    SDL_BindGPUFragmentSamplers(pass, 0, inputs, inputCount);
    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &uniform, sizeof(uniform));
    SDL_DrawGPUPrimitives(pass, FullscreenVertexCount, 1, 0, 0);
    SDL_EndGPURenderPass(pass);
    return true;
}

} // namespace mu
