#pragma once

#include "BitmapGpuResources.h"

namespace mu
{
// Returns a caller-owned texture and a renderer-owned, borrowed sampler.
Render::Renderer::BitmapGpuResources::Result UploadBitmapPixels(std::span<const std::uint8_t> pixels, int width,
                                                                int height, SDL_GPUFilter filter,
                                                                SDL_GPUSamplerAddressMode wrap);
void ReleaseBitmapTexture(SDL_GPUDevice* device, SDL_GPUTexture* texture);
const char* BitmapUploadDriverName();
} // namespace mu
