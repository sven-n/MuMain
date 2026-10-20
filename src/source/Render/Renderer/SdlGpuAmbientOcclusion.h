// SdlGpuAmbientOcclusion.h: screen-space ambient occlusion of the Enhanced scene, from its depth.
#pragma once

#include "SdlGpuPixelFormat.h"

#include <SDL3/SDL_gpu.h>

#include <cstddef>

namespace mu
{

struct SdlGpuAmbientOcclusionShaders
{
    SDL_GPUShader* fullscreenVertex = nullptr;
    SDL_GPUShader* occlusion = nullptr;
    SDL_GPUShader* blur = nullptr;
};

// Mirrors AmbientOcclusion in post_ao.frag.hlsl and post_ao_blur.frag.hlsl. Matrices column-major,
// as glm keeps them.
struct SdlGpuAmbientOcclusionUniform
{
    float projection[16];
    float inverseProjection[16];
    float depthTexelSize[2];
    float radius;
    float padding;
};
static_assert(offsetof(SdlGpuAmbientOcclusionUniform, depthTexelSize) == 128);
static_assert(sizeof(SdlGpuAmbientOcclusionUniform) == 144);

// The occlusion pass at half the frame's size and its blur, into R8 textures the composite reads
// (1: open, 0: hidden). Needs a single-sample, sampleable scene depth.
class SdlGpuAmbientOcclusion
{
public:
    [[nodiscard]] bool CreatePipelines(SDL_GPUDevice* device, const SdlGpuAmbientOcclusionShaders& shaders);
    void DestroyPipelines();
    [[nodiscard]] bool IsAvailable() const;

    // The blurred occlusion of `depth` (of `size`) seen through `projection` (column-major), or null
    // when it cannot be rendered. `radius` is in view-space units.
    [[nodiscard]] SDL_GPUTexture* Render(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture* depth,
                                         SdlGpuTargetSize size, const float (&projection)[16], float radius);

    // Everything, at shutdown.
    void Release();

private:
    [[nodiscard]] bool EnsureTargets(SdlGpuTargetSize size);
    void ReleaseTargets();
    // False when the pass could not be opened.
    [[nodiscard]] bool RunPass(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUGraphicsPipeline* pipeline,
                               SDL_GPUTexture* target, const SDL_GPUTextureSamplerBinding* inputs, Uint32 inputCount,
                               const SdlGpuAmbientOcclusionUniform& uniform);

    SDL_GPUDevice* m_device = nullptr;
    SDL_GPUGraphicsPipeline* m_occlusionPipeline = nullptr;
    SDL_GPUGraphicsPipeline* m_blurPipeline = nullptr;
    SDL_GPUSampler* m_pointSampler = nullptr;
    SdlGpuTargetSize m_size{};
    SDL_GPUTexture* m_occlusion = nullptr;
    SDL_GPUTexture* m_blurred = nullptr;
};

} // namespace mu
