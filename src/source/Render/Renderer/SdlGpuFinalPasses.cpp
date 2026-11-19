#include "SdlGpuFinalPasses.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Render/PostProcessing/EdgeSmoothing.h"

#include <cstddef>

namespace mu
{

namespace
{
using Render::PostProcessing::FinalPassPlan;
using Render::PostProcessing::FrameSize;

// Three vertices cover the target with one triangle (post_fullscreen.vert.hlsl).
constexpr Uint32 FullscreenVertexCount = 3u;
// The images: the composite's and another at the scene's size, and one at the window's.
constexpr std::size_t CompositeImage = 0u;
constexpr std::size_t SceneImage = 1u;
constexpr std::size_t WindowImage = 2u;

struct PipelineTarget
{
    SDL_GPUTextureFormat format = SDL_GPU_TEXTUREFORMAT_INVALID;
    SDL_GPUSampleCount sampleCount = SDL_GPU_SAMPLECOUNT_1;
    bool hasDepth = false;
};

SDL_GPUGraphicsPipeline* BuildPipeline(SDL_GPUDevice* device, SDL_GPUShader* vertexShader,
                                       SDL_GPUShader* fragmentShader, const PipelineTarget& target)
{
    if (!vertexShader || !fragmentShader)
    {
        return nullptr;
    }
    SDL_GPUColorTargetDescription colorTarget{};
    colorTarget.format = target.format;

    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vertexShader;
    info.fragment_shader = fragmentShader;
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    info.multisample_state.sample_count = target.sampleCount;
    info.target_info.color_target_descriptions = &colorTarget;
    info.target_info.num_color_targets = 1;
    info.target_info.has_depth_stencil_target = target.hasDepth;
    info.target_info.depth_stencil_format = SdlGpuDepthFormat;
    return SDL_CreateGPUGraphicsPipeline(device, &info);
}

void ReleasePipeline(SDL_GPUDevice* device, SDL_GPUGraphicsPipeline*& pipeline)
{
    if (pipeline)
    {
        SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
        pipeline = nullptr;
    }
}

std::size_t IndexOf(SdlGpuFinalPasses::Pass pass)
{
    return static_cast<std::size_t>(pass);
}

struct PassList
{
    std::array<SdlGpuFinalPasses::Pass, SdlGpuFinalPasses::PassCount> passes{};
    std::size_t count = 0u;
};

// The passes of `plan`, in the order they run.
PassList PlannedPasses(const FinalPassPlan& plan)
{
    PassList list;
    if (plan.edgeSmoothing)
    {
        list.passes[list.count++] = SdlGpuFinalPasses::Pass::EdgeSmoothing;
    }
    if (plan.upscale)
    {
        list.passes[list.count++] = SdlGpuFinalPasses::Pass::Upscale;
    }
    if (plan.sharpen)
    {
        list.passes[list.count++] = SdlGpuFinalPasses::Pass::Sharpen;
    }
    return list;
}

void SetViewport(SDL_GPURenderPass* pass, FrameSize size)
{
    const SDL_GPUViewport viewport{0.0f, 0.0f, static_cast<float>(size.width), static_cast<float>(size.height),
                                   0.0f, 1.0f};
    const SDL_Rect scissor{0, 0, static_cast<int>(size.width), static_cast<int>(size.height)};
    SDL_SetGPUViewport(pass, &viewport);
    SDL_SetGPUScissor(pass, &scissor);
}

float Inverse(std::uint32_t value)
{
    return 1.0f / static_cast<float>(value);
}

} // namespace

void SdlGpuFinalPasses::CreatePipelines(SDL_GPUDevice* device, const SdlGpuFinalPassShaders& shaders,
                                        SDL_GPUTextureFormat frameFormat, SDL_GPUSampleCount sampleCount)
{
    if (m_device && m_device != device)
    {
        // Handles of an earlier device, destroyed with it: forget them.
        *this = SdlGpuFinalPasses{};
    }
    DestroyPipelines();
    if (frameFormat != m_frameFormat)
    {
        ReleaseImages();
    }
    m_device = device;
    m_frameFormat = frameFormat;
    if (!device)
    {
        return;
    }

    const std::array<SDL_GPUShader*, PassCount> fragmentShaders{shaders.fxaa, shaders.easu, shaders.rcas};
    const PipelineTarget imageTarget{frameFormat};
    const PipelineTarget frameTarget{frameFormat, sampleCount, true};
    for (std::size_t index = 0u; index < PassCount; ++index)
    {
        m_imagePipelines[index] = BuildPipeline(device, shaders.fullscreenVertex, fragmentShaders[index], imageTarget);
        m_framePipelines[index] = BuildPipeline(device, shaders.fullscreenVertex, fragmentShaders[index], frameTarget);
    }
    if (!m_linearSampler)
    {
        CreateSampler();
    }
}

void SdlGpuFinalPasses::CreateSampler()
{
    SDL_GPUSamplerCreateInfo info{};
    info.min_filter = SDL_GPU_FILTER_LINEAR;
    info.mag_filter = SDL_GPU_FILTER_LINEAR;
    info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    m_linearSampler = SDL_CreateGPUSampler(m_device, &info);
}

void SdlGpuFinalPasses::DestroyPipelines()
{
    for (std::size_t index = 0u; index < PassCount; ++index)
    {
        ReleasePipeline(m_device, m_imagePipelines[index]);
        ReleasePipeline(m_device, m_framePipelines[index]);
    }
}

bool SdlGpuFinalPasses::IsAvailable(Pass pass) const
{
    const std::size_t index = IndexOf(pass);
    return m_linearSampler && m_imagePipelines[index] && m_framePipelines[index];
}

bool SdlGpuFinalPasses::CanRun(const FinalPassPlan& plan) const
{
    return (!plan.edgeSmoothing || IsAvailable(Pass::EdgeSmoothing)) && (!plan.upscale || IsAvailable(Pass::Upscale)) &&
           (!plan.sharpen || IsAvailable(Pass::Sharpen));
}

SdlGpuFinalPasses::Image* SdlGpuFinalPasses::EnsureImage(std::size_t index, FrameSize size)
{
    Image& image = m_images[index];
    if (image.texture && image.size == size)
    {
        return &image;
    }
    if (image.texture)
    {
        SDL_ReleaseGPUTexture(m_device, image.texture);
        image = {};
    }
    if (!m_device || size.width == 0u || size.height == 0u)
    {
        return nullptr;
    }

    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = m_frameFormat;
    info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    info.width = size.width;
    info.height = size.height;
    info.layer_count_or_depth = 1u;
    info.num_levels = 1u;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    image.texture = SDL_CreateGPUTexture(m_device, &info);
    if (!image.texture)
    {
        log::Get("render")->error("SDL_gpu -- final pass image failed ({}x{}): {}", size.width, size.height,
                                  SDL_GetError());
        return nullptr;
    }
    image.size = size;
    return &image;
}

SDL_GPUTexture* SdlGpuFinalPasses::EnsureCompositeImage(FrameSize sceneSize)
{
    const Image* image = EnsureImage(CompositeImage, sceneSize);
    return image ? image->texture : nullptr;
}

bool SdlGpuFinalPasses::RunAllButLast(SDL_GPUCommandBuffer* commandBuffer, const FinalPassPlan& plan,
                                      FrameSize sceneSize, FrameSize windowSize, int sharpnessPercent)
{
    m_sharpnessPercent = sharpnessPercent;
    m_last = {};
    const PassList list = PlannedPasses(plan);
    const Image* source = &m_images[CompositeImage];
    if (list.count == 0u || !CanRun(plan) || !source->texture || source->size != sceneSize)
    {
        return false;
    }

    for (std::size_t index = 0u; index < list.count; ++index)
    {
        const Pass pass = list.passes[index];
        const Step step{pass, source, pass == Pass::Upscale ? windowSize : source->size};
        if (index + 1u == list.count)
        {
            m_last = step;
            return true;
        }
        const Image* output = RunIntoImage(commandBuffer, step, sceneSize);
        if (!output)
        {
            return false;
        }
        source = output;
    }
    return false;
}

const SdlGpuFinalPasses::Image* SdlGpuFinalPasses::RunIntoImage(SDL_GPUCommandBuffer* commandBuffer, const Step& step,
                                                                FrameSize sceneSize)
{
    // Never the image the pass reads: at the scene's size, the composite's is read first.
    const bool sceneSized = step.outputSize == sceneSize && step.source != &m_images[SceneImage];
    const Image* output = EnsureImage(sceneSized ? SceneImage : WindowImage, step.outputSize);
    if (!output || output == step.source)
    {
        return nullptr;
    }

    SDL_GPUColorTargetInfo colorTarget{};
    colorTarget.texture = output->texture;
    colorTarget.load_op = SDL_GPU_LOADOP_DONT_CARE;
    colorTarget.store_op = SDL_GPU_STOREOP_STORE;
    // Not loaded: replaced by a fresh texture instead of waiting for an earlier frame.
    colorTarget.cycle = true;
    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTarget, 1, nullptr);
    if (!renderPass)
    {
        return nullptr;
    }
    Draw(commandBuffer, renderPass, m_imagePipelines[IndexOf(step.pass)], step);
    SDL_EndGPURenderPass(renderPass);
    return output;
}

void SdlGpuFinalPasses::DrawLast(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* pass)
{
    if (m_last.source && m_last.source->texture)
    {
        Draw(commandBuffer, pass, m_framePipelines[IndexOf(m_last.pass)], m_last);
    }
}

void SdlGpuFinalPasses::Draw(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* pass,
                             SDL_GPUGraphicsPipeline* pipeline, const Step& step) const
{
    SetViewport(pass, step.outputSize);
    SDL_BindGPUGraphicsPipeline(pass, pipeline);
    const SDL_GPUTextureSamplerBinding binding{step.source->texture, m_linearSampler};
    SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);
    PushUniform(commandBuffer, step);
    SDL_DrawGPUPrimitives(pass, FullscreenVertexCount, 1, 0, 0);
}

void SdlGpuFinalPasses::PushUniform(SDL_GPUCommandBuffer* commandBuffer, const Step& step) const
{
    const FrameSize input = step.source->size;
    switch (step.pass)
    {
    case Pass::EdgeSmoothing:
    {
        const SdlGpuFxaaUniform uniform{
            {Inverse(input.width), Inverse(input.height)}, Render::PostProcessing::EdgeSmoothingSubpixelStrength, 0.0f};
        SDL_PushGPUFragmentUniformData(commandBuffer, 0, &uniform, sizeof(uniform));
        return;
    }
    case Pass::Upscale:
    {
        const SdlGpuEasuUniform uniform{Render::PostProcessing::EasuConstants(input, step.outputSize)};
        SDL_PushGPUFragmentUniformData(commandBuffer, 0, &uniform, sizeof(uniform));
        return;
    }
    case Pass::Sharpen:
    {
        const SdlGpuRcasUniform uniform{Render::PostProcessing::RcasSharpness(m_sharpnessPercent),
                                        static_cast<float>(input.width - 1u), static_cast<float>(input.height - 1u),
                                        0.0f};
        SDL_PushGPUFragmentUniformData(commandBuffer, 0, &uniform, sizeof(uniform));
        return;
    }
    }
}

void SdlGpuFinalPasses::ReleaseImages()
{
    for (Image& image : m_images)
    {
        if (image.texture)
        {
            SDL_ReleaseGPUTexture(m_device, image.texture);
        }
        image = {};
    }
    m_last = {};
}

void SdlGpuFinalPasses::Release()
{
    ReleaseImages();
    DestroyPipelines();
    if (m_linearSampler)
    {
        SDL_ReleaseGPUSampler(m_device, m_linearSampler);
        m_linearSampler = nullptr;
    }
    m_device = nullptr;
}

} // namespace mu
