#pragma once

#include "BitmapGpuResources.h"

namespace mu
{
// Returns a caller-owned texture and a renderer-owned, borrowed sampler.
// `mips` are the levels below `pixels` (see BitmapGpuResources::Upload).
Render::Renderer::BitmapGpuResources::Result UploadBitmapPixels(std::span<const std::uint8_t> pixels, int width,
                                                                int height, SDL_GPUFilter filter,
                                                                SDL_GPUSamplerAddressMode wrap,
                                                                std::span<const Render::Textures::MipLevel> mips = {},
                                                                bool generateMips = false);
void ReleaseBitmapTexture(SDL_GPUDevice* device, SDL_GPUTexture* texture);
const char* BitmapUploadDriverName();
double BitmapUploadFenceWaitSeconds();
// Whether a staged load should leave its next step to the next frame (BitmapGpuResources::OverStagingCap).
bool BitmapUploadsOverStagingCap();
// Submits the staged uploads now: the end of a load, so its textures are on the GPU without waiting for a frame.
void FlushBitmapUploads();
} // namespace mu
