// SdlGpuEdgeSmoothing.h: edge smoothing (FXAA) of the processed 3D scene, before the UI.
#pragma once

#include "SdlGpuPixelFormat.h"

#include <SDL3/SDL_gpu.h>

#include <cstddef>

namespace mu
{

struct SdlGpuEdgeSmoothingShaders
{
    SDL_GPUShader* fullscreenVertex = nullptr;
    SDL_GPUShader* fxaa = nullptr;
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

// The processed image the composite draws into when edge smoothing is on, and the pass that draws
// it smoothed into the frame's UI pass, where the UI then goes on top. Full-screen only: one texture
// and one sampler per frame, whatever the scene holds.
class SdlGpuEdgeSmoothing
{
public:
    // Builds the pass for frames of `frameFormat` with `sampleCount` samples per pixel. It draws
    // inside the UI pass, which has a depth target. False when it cannot: edge smoothing is then
    // unavailable.
    [[nodiscard]] bool CreatePipelines(SDL_GPUDevice* device, const SdlGpuEdgeSmoothingShaders& shaders,
                                       SDL_GPUTextureFormat frameFormat, SDL_GPUSampleCount sampleCount);
    void DestroyPipelines();
    [[nodiscard]] bool IsAvailable() const;

    // The processed image of `size` in the frame's format, recreated when the size changed; null
    // when it cannot be created.
    [[nodiscard]] SDL_GPUTexture* EnsureImage(SdlGpuTargetSize size);

    // Draws the processed image, smoothed, over the whole target of `pass`.
    void Draw(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* pass, SdlGpuTargetSize size,
              float subpixelStrength);

    // The image, while edge smoothing is off.
    void ReleaseImage();
    // Everything, at shutdown.
    void Release();

private:
    SDL_GPUDevice* m_device = nullptr;
    SDL_GPUTextureFormat m_frameFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
    SDL_GPUGraphicsPipeline* m_pipeline = nullptr;
    SDL_GPUSampler* m_linearSampler = nullptr;
    SdlGpuTargetSize m_size{};
    SDL_GPUTexture* m_image = nullptr;
};

} // namespace mu
