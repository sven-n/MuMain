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

} // namespace mu
