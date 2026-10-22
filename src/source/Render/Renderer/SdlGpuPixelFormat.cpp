#include "SdlGpuPixelFormat.h"

namespace mu
{

std::optional<PixelChannelOrder> GetSdlGpuPixelChannelOrder(SDL_GPUTextureFormat format)
{
    switch (format)
    {
    case SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM:
    case SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB:
        return PixelChannelOrder::Rgba;
    case SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM:
    case SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB:
        return PixelChannelOrder::Bgra;
    default:
        return std::nullopt;
    }
}

std::optional<SDL_GPUTextureCreateInfo> GetSdlGpuFrameCaptureTextureInfo(
    SDL_GPUTextureFormat format, Uint32 width, Uint32 height)
{
    if (width == 0u || height == 0u || !GetSdlGpuPixelChannelOrder(format))
    {
        return std::nullopt;
    }

    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = format;
    info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    info.width = width;
    info.height = height;
    info.layer_count_or_depth = 1u;
    info.num_levels = 1u;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    return info;
}

std::optional<SDL_GPUSampleCount> GetSdlGpuSampleCount(int samples)
{
    switch (samples)
    {
    case 1:
        return SDL_GPU_SAMPLECOUNT_1;
    case 2:
        return SDL_GPU_SAMPLECOUNT_2;
    case 4:
        return SDL_GPU_SAMPLECOUNT_4;
    case 8:
        return SDL_GPU_SAMPLECOUNT_8;
    default:
        return std::nullopt;
    }
}

std::optional<SDL_GPUTextureCreateInfo>
GetSdlGpuMultisampleColorTextureInfo(SDL_GPUTextureFormat format, SdlGpuTargetSize size, SDL_GPUSampleCount sampleCount)
{
    if (sampleCount == SDL_GPU_SAMPLECOUNT_1 || size.width == 0u || size.height == 0u ||
        !GetSdlGpuPixelChannelOrder(format))
    {
        return std::nullopt;
    }

    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = format;
    info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    info.width = size.width;
    info.height = size.height;
    info.layer_count_or_depth = 1u;
    info.num_levels = 1u;
    info.sample_count = sampleCount;
    return info;
}

std::optional<SDL_GPUTextureCreateInfo> GetSdlGpuDepthTextureInfo(SdlGpuTargetSize size, SDL_GPUSampleCount sampleCount)
{
    if (size.width == 0u || size.height == 0u)
    {
        return std::nullopt;
    }

    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SdlGpuDepthFormat;
    info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    info.width = size.width;
    info.height = size.height;
    info.layer_count_or_depth = 1u;
    info.num_levels = 1u;
    info.sample_count = sampleCount;
    return info;
}

SDL_GPUColorTargetInfo GetSdlGpuFrameColorTarget(SdlGpuFrameTargets targets, SDL_FColor clearColor)
{
    SDL_GPUColorTargetInfo colorTarget{};
    colorTarget.clear_color = clearColor;
    colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
    if (targets.multisampled)
    {
        // RESOLVE lets the driver discard the samples once they are resolved.
        colorTarget.texture = targets.multisampled;
        colorTarget.store_op = SDL_GPU_STOREOP_RESOLVE;
        colorTarget.resolve_texture = targets.destination;
    }
    else
    {
        colorTarget.texture = targets.destination;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;
    }
    return colorTarget;
}

} // namespace mu
