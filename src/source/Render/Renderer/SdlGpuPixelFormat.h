#pragma once

#include "FramePixelReadback.h"

#include <SDL3/SDL_gpu.h>

#include <optional>

namespace mu
{

[[nodiscard]] std::optional<PixelChannelOrder> GetSdlGpuPixelChannelOrder(SDL_GPUTextureFormat format);
[[nodiscard]] std::optional<SDL_GPUTextureCreateInfo> GetSdlGpuFrameCaptureTextureInfo(
    SDL_GPUTextureFormat format, Uint32 width, Uint32 height);

} // namespace mu
