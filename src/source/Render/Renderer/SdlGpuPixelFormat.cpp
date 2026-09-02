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

} // namespace mu
