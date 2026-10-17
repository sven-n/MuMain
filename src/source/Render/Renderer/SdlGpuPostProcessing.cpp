#include "SdlGpuPostProcessing.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Render/PostProcessing/BloomComposite.h"

#include <algorithm>
#include <cstring>

namespace mu
{

namespace
{
// Three vertices cover the target with one triangle (post_fullscreen.vert.hlsl).
constexpr Uint32 FullscreenVertexCount = 3u;
// Float bloom levels keep dim glows from banding and let the upsample passes sum
// above 1 before the composite scales the result down.
constexpr SDL_GPUTextureFormat PreferredBloomFormat = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
using Render::PostProcessing::GradingTableBytesPerTexel;
using Render::PostProcessing::GradingTableHeight;
using Render::PostProcessing::GradingTableWidth;
constexpr SDL_FColor TransparentBlack{0.0f, 0.0f, 0.0f, 0.0f};

struct PipelineDescription
{
    SDL_GPUShader* fragmentShader = nullptr;
    SDL_GPUTextureFormat colorFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
    SDL_GPUSampleCount sampleCount = SDL_GPU_SAMPLECOUNT_1;
    bool additive = false;
    bool hasDepthTarget = false;
};

SDL_GPUGraphicsPipeline* BuildPipeline(SDL_GPUDevice* device, SDL_GPUShader* vertexShader,
                                       const PipelineDescription& description)
{
    SDL_GPUColorTargetDescription colorTarget{};
    colorTarget.format = description.colorFormat;
    if (description.additive)
    {
        colorTarget.blend_state.enable_blend = true;
        colorTarget.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        colorTarget.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        colorTarget.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        colorTarget.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        colorTarget.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        colorTarget.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    }

    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vertexShader;
    info.fragment_shader = description.fragmentShader;
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    info.multisample_state.sample_count = description.sampleCount;
    info.target_info.color_target_descriptions = &colorTarget;
    info.target_info.num_color_targets = 1;
    info.target_info.has_depth_stencil_target = description.hasDepthTarget;
    info.target_info.depth_stencil_format = SdlGpuDepthFormat;
    return SDL_CreateGPUGraphicsPipeline(device, &info);
}

SDL_GPUTexture* CreateTarget(SDL_GPUDevice* device, SDL_GPUTextureFormat format, SdlGpuTargetSize size)
{
    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = format;
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

SdlGpuTargetSize HalfSize(SdlGpuTargetSize size)
{
    return {std::max(size.width / 2u, 1u), std::max(size.height / 2u, 1u)};
}

SdlGpuBloomUniform MakeBloomUniform(SdlGpuTargetSize sourceSize, float levelWeight)
{
    return {
        {1.0f / static_cast<float>(sourceSize.width), 1.0f / static_cast<float>(sourceSize.height)}, levelWeight, 0.0f};
}

} // namespace

bool SdlGpuPostProcessing::CreatePipelines(SDL_GPUDevice* device, const SdlGpuPostShaders& shaders,
                                           SDL_GPUTextureFormat frameFormat, SDL_GPUSampleCount sampleCount)
{
    if (m_device && m_device != device)
    {
        // Handles of an earlier device, destroyed by a failed start-up together
        // with everything it owned: forget them, there is nothing left to release.
        *this = SdlGpuPostProcessing{};
    }
    DestroyPipelines();
    if (!device || !shaders.fullscreenVertex || !shaders.bloomDown || !shaders.bloomUp || !shaders.composite)
    {
        return false;
    }
    m_device = device;

    const SDL_GPUTextureUsageFlags targetUsage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    const SDL_GPUTextureFormat bloomFormat =
        SDL_GPUTextureSupportsFormat(device, PreferredBloomFormat, SDL_GPU_TEXTURETYPE_2D, targetUsage)
            ? PreferredBloomFormat
            : frameFormat;
    if (bloomFormat != m_bloomFormat || frameFormat != m_frameFormat)
    {
        ReleaseTargets();
    }
    m_frameFormat = frameFormat;
    m_bloomFormat = bloomFormat;

    m_bloomDownPipeline = BuildPipeline(device, shaders.fullscreenVertex, {shaders.bloomDown, bloomFormat});
    m_bloomUpPipeline = BuildPipeline(device, shaders.fullscreenVertex, {shaders.bloomUp, bloomFormat, {}, true});
    m_compositePipeline =
        BuildPipeline(device, shaders.fullscreenVertex, {shaders.composite, frameFormat, sampleCount, false, true});
    if (!m_bloomDownPipeline || !m_bloomUpPipeline || !m_compositePipeline || !CreateSamplers() ||
        !CreateGradingTexture())
    {
        log::Get("render")->error("SDL_gpu -- post-processing pipelines failed: {}", SDL_GetError());
        DestroyPipelines();
        return false;
    }
    return true;
}

void SdlGpuPostProcessing::DestroyPipelines()
{
    ReleasePipeline(m_device, m_bloomDownPipeline);
    ReleasePipeline(m_device, m_bloomUpPipeline);
    ReleasePipeline(m_device, m_compositePipeline);
}

bool SdlGpuPostProcessing::IsAvailable() const
{
    return m_bloomDownPipeline && m_bloomUpPipeline && m_compositePipeline && m_pointSampler && m_linearSampler &&
           m_gradingTexture;
}

bool SdlGpuPostProcessing::CreateSamplers()
{
    if (m_pointSampler && m_linearSampler)
    {
        return true;
    }

    SDL_GPUSamplerCreateInfo info{};
    info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    if (!m_pointSampler)
    {
        info.min_filter = SDL_GPU_FILTER_NEAREST;
        info.mag_filter = SDL_GPU_FILTER_NEAREST;
        m_pointSampler = SDL_CreateGPUSampler(m_device, &info);
    }
    if (!m_linearSampler)
    {
        info.min_filter = SDL_GPU_FILTER_LINEAR;
        info.mag_filter = SDL_GPU_FILTER_LINEAR;
        m_linearSampler = SDL_CreateGPUSampler(m_device, &info);
    }
    return m_pointSampler && m_linearSampler;
}

bool SdlGpuPostProcessing::CreateGradingTexture()
{
    if (m_gradingTexture)
    {
        return true;
    }

    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    info.width = GradingTableWidth;
    info.height = GradingTableHeight;
    info.layer_count_or_depth = 1u;
    info.num_levels = 1u;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    m_gradingTexture = SDL_CreateGPUTexture(m_device, &info);
    m_gradingEnabled = false;
    return m_gradingTexture != nullptr;
}

bool SdlGpuPostProcessing::EnsureTargets(SdlGpuTargetSize size)
{
    if (!m_device || size.width == 0u || size.height == 0u)
    {
        return false;
    }
    if (m_sceneTexture && m_targetSize.width == size.width && m_targetSize.height == size.height)
    {
        return true;
    }

    ReleaseTargets();
    m_sceneTexture = CreateTarget(m_device, m_frameFormat, size);
    m_bloomSourceTexture = CreateTarget(m_device, m_frameFormat, size);
    SdlGpuTargetSize levelSize = HalfSize(size);
    bool created = m_sceneTexture && m_bloomSourceTexture;
    for (BloomLevel& level : m_bloomLevels)
    {
        level.size = levelSize;
        level.texture = CreateTarget(m_device, m_bloomFormat, levelSize);
        created = created && level.texture;
        levelSize = HalfSize(levelSize);
    }
    if (!created)
    {
        log::Get("render")->error("SDL_gpu -- post-processing targets failed ({}x{}): {}", size.width, size.height,
                                  SDL_GetError());
        ReleaseTargets();
        return false;
    }
    m_targetSize = size;
    return true;
}

SDL_GPUTexture* SdlGpuPostProcessing::GetSceneTexture() const
{
    return m_sceneTexture;
}

SDL_GPUTexture* SdlGpuPostProcessing::GetBloomSourceTexture() const
{
    return m_bloomSourceTexture;
}

void SdlGpuPostProcessing::ReleaseTargets()
{
    ReleaseTexture(m_device, m_sceneTexture);
    ReleaseTexture(m_device, m_bloomSourceTexture);
    for (BloomLevel& level : m_bloomLevels)
    {
        ReleaseTexture(m_device, level.texture);
        level.size = {};
    }
    m_targetSize = {};
    m_bloomRendered = false;
}

void SdlGpuPostProcessing::RunBloomPass(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUGraphicsPipeline* pipeline,
                                        const BloomLevel& target, SDL_GPULoadOp loadOp, SDL_GPUTexture* source,
                                        SdlGpuTargetSize sourceSize, float levelWeight)
{
    SDL_GPUColorTargetInfo colorTarget{};
    colorTarget.texture = target.texture;
    colorTarget.clear_color = TransparentBlack;
    colorTarget.load_op = loadOp;
    colorTarget.store_op = SDL_GPU_STOREOP_STORE;
    // Not loaded: replaced by a fresh texture instead of waiting for an earlier frame.
    colorTarget.cycle = loadOp != SDL_GPU_LOADOP_LOAD;
    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(commandBuffer, &colorTarget, 1, nullptr);
    if (!pass)
    {
        return;
    }

    const SDL_GPUViewport viewport{
        0.0f, 0.0f, static_cast<float>(target.size.width), static_cast<float>(target.size.height), 0.0f, 1.0f};
    const SDL_Rect scissor{0, 0, static_cast<int>(target.size.width), static_cast<int>(target.size.height)};
    SDL_SetGPUViewport(pass, &viewport);
    SDL_SetGPUScissor(pass, &scissor);
    SDL_BindGPUGraphicsPipeline(pass, pipeline);
    const SDL_GPUTextureSamplerBinding binding{source, m_linearSampler};
    SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);
    const SdlGpuBloomUniform uniform = MakeBloomUniform(sourceSize, levelWeight);
    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &uniform, sizeof(uniform));
    SDL_DrawGPUPrimitives(pass, FullscreenVertexCount, 1, 0, 0);
    SDL_EndGPURenderPass(pass);
}

void SdlGpuPostProcessing::RenderBloom(SDL_GPUCommandBuffer* commandBuffer, float levelFalloff)
{
    if (!IsAvailable() || !m_bloomSourceTexture)
    {
        return;
    }

    // Down: the bloom source into the first level, each level into the next.
    SDL_GPUTexture* source = m_bloomSourceTexture;
    SdlGpuTargetSize sourceSize = m_targetSize;
    for (const BloomLevel& level : m_bloomLevels)
    {
        RunBloomPass(commandBuffer, m_bloomDownPipeline, level, SDL_GPU_LOADOP_DONT_CARE, source, sourceSize, 1.0f);
        source = level.texture;
        sourceSize = level.size;
    }

    // Up: each level is blurred into the next larger one and added to it at
    // `levelFalloff`, so the first level ends up with the glow of every radius,
    // the wide ones fainter.
    for (std::size_t index = BloomLevelCount - 1u; index > 0u; --index)
    {
        const BloomLevel& smaller = m_bloomLevels[index];
        RunBloomPass(commandBuffer, m_bloomUpPipeline, m_bloomLevels[index - 1u], SDL_GPU_LOADOP_LOAD, smaller.texture,
                     smaller.size, levelFalloff);
    }
    m_bloomRendered = true;
}

void SdlGpuPostProcessing::DrawComposite(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* pass,
                                         SdlGpuTargetSize size, const PostProcessingLook& look, PostProcessingView view)
{
    if (!IsAvailable() || !m_sceneTexture || size.width == 0u || size.height == 0u)
    {
        return;
    }

    const bool bloomRendered = m_bloomRendered && look.bloomStrength > 0.0f;
    m_bloomRendered = false;

    // The image shown as the scene: the scene itself, or a diagnostic view in
    // its place (black when the bloom was not rendered this frame).
    SDL_GPUTexture* shown = m_sceneTexture;
    SDL_GPUSampler* shownSampler = m_pointSampler;
    bool bloom = bloomRendered;
    const float gradingStrength = Render::PostProcessing::CompositeGradingStrength(
        m_gradingEnabled, view != PostProcessingView::Scene, look.gradingStrength);
    const bool grading = gradingStrength > 0.0f;
    if (view != PostProcessingView::Scene)
    {
        bloom = false;
        if (!bloomRendered)
        {
            shown = nullptr;
        }
        else if (view == PostProcessingView::BloomSource)
        {
            shown = m_bloomSourceTexture;
        }
        else
        {
            shown = m_bloomLevels[0].texture;
            shownSampler = m_linearSampler;
        }
    }
    if (!shown)
    {
        return; // the pass's clear colour stays
    }

    // Every level of the chain adds its weighted blur of the source to the first one.
    const float bloomScale =
        bloom ? look.bloomStrength / Render::PostProcessing::BloomWeightSum(BloomLevelCount, look.bloomLevelFalloff)
              : 0.0f;
    // Unused inputs still need a binding: the scene texture stands in for them.
    const SDL_GPUTextureSamplerBinding bindings[] = {
        {shown, shownSampler},
        {bloom ? m_bloomLevels[0].texture : m_sceneTexture, m_linearSampler},
        {grading ? m_gradingTexture : m_sceneTexture, m_linearSampler},
    };

    const SDL_GPUViewport viewport{0.0f, 0.0f, static_cast<float>(size.width), static_cast<float>(size.height),
                                   0.0f, 1.0f};
    const SDL_Rect scissor{0, 0, static_cast<int>(size.width), static_cast<int>(size.height)};
    SDL_SetGPUViewport(pass, &viewport);
    SDL_SetGPUScissor(pass, &scissor);
    SDL_BindGPUGraphicsPipeline(pass, m_compositePipeline);
    SDL_BindGPUFragmentSamplers(pass, 0, bindings, static_cast<Uint32>(std::size(bindings)));
    const SdlGpuCompositeUniform uniform{
        {1.0f / static_cast<float>(size.width), 1.0f / static_cast<float>(size.height)}, bloomScale, gradingStrength};
    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &uniform, sizeof(uniform));
    SDL_DrawGPUPrimitives(pass, FullscreenVertexCount, 1, 0, 0);
}

bool SdlGpuPostProcessing::SetGradingTable(std::span<const std::uint8_t> rgba)
{
    m_gradingEnabled = false;
    if (rgba.empty())
    {
        return true;
    }

    const Uint32 byteCount = GradingTableWidth * GradingTableHeight * GradingTableBytesPerTexel;
    if (rgba.size() != byteCount || !m_device || !CreateGradingTexture())
    {
        return false;
    }

    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = byteCount;
    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(m_device, &transferInfo);
    if (!transfer)
    {
        return false;
    }
    void* mapped = SDL_MapGPUTransferBuffer(m_device, transfer, false);
    if (!mapped)
    {
        SDL_ReleaseGPUTransferBuffer(m_device, transfer);
        return false;
    }
    std::memcpy(mapped, rgba.data(), byteCount);
    SDL_UnmapGPUTransferBuffer(m_device, transfer);

    // Its own command buffer, submitted at once: it runs before the frame being
    // recorded, which is submitted later.
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(m_device);
    SDL_GPUCopyPass* copyPass = commandBuffer ? SDL_BeginGPUCopyPass(commandBuffer) : nullptr;
    if (!copyPass)
    {
        if (commandBuffer)
        {
            SDL_CancelGPUCommandBuffer(commandBuffer);
        }
        SDL_ReleaseGPUTransferBuffer(m_device, transfer);
        return false;
    }

    SDL_GPUTextureTransferInfo source{};
    source.transfer_buffer = transfer;
    source.pixels_per_row = GradingTableWidth;
    source.rows_per_layer = GradingTableHeight;
    SDL_GPUTextureRegion destination{};
    destination.texture = m_gradingTexture;
    destination.w = GradingTableWidth;
    destination.h = GradingTableHeight;
    destination.d = 1u;
    // Cycling leaves the previous table to frames still in flight.
    SDL_UploadToGPUTexture(copyPass, &source, &destination, true);
    SDL_EndGPUCopyPass(copyPass);
    const bool submitted = SDL_SubmitGPUCommandBuffer(commandBuffer);
    SDL_ReleaseGPUTransferBuffer(m_device, transfer);
    m_gradingEnabled = submitted;
    return submitted;
}

bool SdlGpuPostProcessing::HasGradingTable() const
{
    return m_gradingEnabled;
}

void SdlGpuPostProcessing::Release()
{
    DestroyPipelines();
    ReleaseTargets();
    ReleaseTexture(m_device, m_gradingTexture);
    m_gradingEnabled = false;
    if (m_pointSampler)
    {
        SDL_ReleaseGPUSampler(m_device, m_pointSampler);
        m_pointSampler = nullptr;
    }
    if (m_linearSampler)
    {
        SDL_ReleaseGPUSampler(m_device, m_linearSampler);
        m_linearSampler = nullptr;
    }
    m_device = nullptr;
    m_frameFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
    m_bloomFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
}

} // namespace mu
