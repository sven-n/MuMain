// SdlGpuFinalPasses.h: the passes between the composite of the processed 3D scene and the UI: edge
// smoothing (FXAA), upscaling (FSR 1 EASU) and sharpening (FSR 1 RCAS).
#pragma once

#include "SdlGpuPixelFormat.h"
#include "Render/PostProcessing/RenderScale.h"

#include <SDL3/SDL_gpu.h>

#include <array>
#include <cstddef>

namespace mu
{

struct SdlGpuFinalPassShaders
{
    SDL_GPUShader* fullscreenVertex = nullptr;
    SDL_GPUShader* fxaa = nullptr;
    SDL_GPUShader* easu = nullptr;
    SDL_GPUShader* rcas = nullptr;
};

// Mirrors FxaaUniforms in post_fxaa.frag.hlsl.
struct SdlGpuFxaaUniform
{
    float texelSize[2];
    float subpixelStrength;
    float padding;
};
static_assert(offsetof(SdlGpuFxaaUniform, subpixelStrength) == 8);
static_assert(sizeof(SdlGpuFxaaUniform) == 16);

// Mirrors EasuUniforms in post_fsr_easu.frag.hlsl (Render::PostProcessing::EasuConstants).
struct SdlGpuEasuUniform
{
    std::array<float, 16> constants;
};
static_assert(sizeof(SdlGpuEasuUniform) == 64);

// Mirrors RcasUniforms in post_fsr_rcas.frag.hlsl.
struct SdlGpuRcasUniform
{
    float sharpness;
    float maxX;
    float maxY;
    float padding;
};
static_assert(sizeof(SdlGpuRcasUniform) == 16);

// The composite draws into this class's first image; each planned pass then reads the previous
// image, all but the last into another image, and the last inside the frame's UI pass, where the UI
// then goes on top. Each pass is one full-screen draw with one texture and one sampler.
class SdlGpuFinalPasses
{
public:
    enum class Pass : unsigned char
    {
        EdgeSmoothing,
        Upscale,
        Sharpen,
    };
    static constexpr std::size_t PassCount = 3u;

    // Builds each pass twice, for frames of `frameFormat`: into an image, and inside the UI pass
    // with `sampleCount` samples and a depth target. A pass whose pipelines fail is unavailable.
    void CreatePipelines(SDL_GPUDevice* device, const SdlGpuFinalPassShaders& shaders, SDL_GPUTextureFormat frameFormat,
                         SDL_GPUSampleCount sampleCount);
    void DestroyPipelines();
    [[nodiscard]] bool IsAvailable(Pass pass) const;
    // Every pass `plan` asks for is available.
    [[nodiscard]] bool CanRun(const Render::PostProcessing::FinalPassPlan& plan) const;

    // The image of `sceneSize` the composite draws into; null when it cannot be created.
    [[nodiscard]] SDL_GPUTexture* EnsureCompositeImage(Render::PostProcessing::FrameSize sceneSize);

    // The passes of `plan` but its last, each into an image; the scene is `sceneSize`, the window
    // `windowSize`. False when one could not run: the frame then shows the composite instead.
    [[nodiscard]] bool RunAllButLast(SDL_GPUCommandBuffer* commandBuffer,
                                     const Render::PostProcessing::FinalPassPlan& plan,
                                     Render::PostProcessing::FrameSize sceneSize,
                                     Render::PostProcessing::FrameSize windowSize, int sharpnessPercent);
    // The last pass of the plan RunAllButLast ran, over the whole target of `pass`.
    void DrawLast(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* pass);

    // The images, while no pass runs.
    void ReleaseImages();
    // Everything, at shutdown.
    void Release();

private:
    struct Image
    {
        SDL_GPUTexture* texture = nullptr;
        Render::PostProcessing::FrameSize size{};
    };
    // A planned pass: what it reads, at which size, and what it draws at which size.
    struct Step
    {
        Pass pass = Pass::EdgeSmoothing;
        const Image* source = nullptr;
        Render::PostProcessing::FrameSize outputSize{};
    };
    static constexpr std::size_t ImageCount = 3u; // two at the scene's size, one at the window's

    void CreateSampler();
    [[nodiscard]] Image* EnsureImage(std::size_t index, Render::PostProcessing::FrameSize size);
    // `step` into an image of its output size that it does not read; null when it cannot run.
    [[nodiscard]] const Image* RunIntoImage(SDL_GPUCommandBuffer* commandBuffer, const Step& step,
                                            Render::PostProcessing::FrameSize sceneSize);
    void Draw(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* pass, SDL_GPUGraphicsPipeline* pipeline,
              const Step& step) const;
    void PushUniform(SDL_GPUCommandBuffer* commandBuffer, const Step& step) const;

    SDL_GPUDevice* m_device = nullptr;
    SDL_GPUTextureFormat m_frameFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
    std::array<SDL_GPUGraphicsPipeline*, PassCount> m_imagePipelines{};
    std::array<SDL_GPUGraphicsPipeline*, PassCount> m_framePipelines{};
    SDL_GPUSampler* m_linearSampler = nullptr;
    std::array<Image, ImageCount> m_images{};
    Step m_last{};
    int m_sharpnessPercent = 0;
};

} // namespace mu
