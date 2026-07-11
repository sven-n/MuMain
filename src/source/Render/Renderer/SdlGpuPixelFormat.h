#pragma once

#include "FramePixelReadback.h"

#include <SDL3/SDL_gpu.h>

#include <optional>

namespace mu
{

[[nodiscard]] std::optional<PixelChannelOrder> GetSdlGpuPixelChannelOrder(SDL_GPUTextureFormat format);

} // namespace mu
