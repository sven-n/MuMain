#pragma once

#include "FramePixelReadback.h"

#include <SDL3/SDL_gpu.h>

#include <optional>

namespace mu
{

[[nodiscard]] std::optional<PixelChannelOrder> GetSdlGpuPixelChannelOrder(SDL_GPUTextureFormat format);
[[nodiscard]] std::optional<SDL_GPUTextureCreateInfo> GetSdlGpuFrameCaptureTextureInfo(
    SDL_GPUTextureFormat format, Uint32 width, Uint32 height);

// Depth format of every render pass.
inline constexpr SDL_GPUTextureFormat SdlGpuDepthFormat = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;

struct SdlGpuTargetSize
{
    Uint32 width = 0u;
    Uint32 height = 0u;
};

// SDL's sample count for 1, 2, 4 or 8 samples per pixel; nullopt otherwise.
[[nodiscard]] std::optional<SDL_GPUSampleCount> GetSdlGpuSampleCount(int samples);

// The colour target a multisampled frame renders into before the pass
// resolves it into the frame's destination: the destination's format and
// size, render-target use only. nullopt for one sample, an empty size or a
// format frames are never read back in.
[[nodiscard]] std::optional<SDL_GPUTextureCreateInfo>
GetSdlGpuMultisampleColorTextureInfo(SDL_GPUTextureFormat format, SdlGpuTargetSize size,
                                     SDL_GPUSampleCount sampleCount);

// The frame's depth buffer, with the frame's sample count.
[[nodiscard]] std::optional<SDL_GPUTextureCreateInfo> GetSdlGpuDepthTextureInfo(SdlGpuTargetSize size,
                                                                                SDL_GPUSampleCount sampleCount);

struct SdlGpuFrameTargets
{
    // Where the finished frame must end up: the swapchain texture, or the
    // readback or capture texture that is later copied to the swapchain.
    SDL_GPUTexture* destination = nullptr;
    // The multisampled colour target, or null when the frame has one sample.
    SDL_GPUTexture* multisampled = nullptr;
};

// The main pass's colour target. With one sample it renders straight into the
// destination; multisampled, it renders into the multisampled target and the
// pass resolves it into the destination, so screenshots and readback, which
// read the destination, always get the resolved single-sample image.
[[nodiscard]] SDL_GPUColorTargetInfo GetSdlGpuFrameColorTarget(SdlGpuFrameTargets targets, SDL_FColor clearColor);

} // namespace mu
