#pragma once

namespace Render::Sprites
{
// Whole-image RGBA uploads require a fixed staging allocation sized from these
// decoder limits. Raising either limit also raises the per-slot GPU budget.
inline constexpr int kBitmapMaximumWidth = 1024;
inline constexpr int kBitmapMaximumHeight = 1024;
// Upscaled art (Render/Textures/UpscaleMarker.h) is up to this times the limits
// above; such an image is staged in bands of rows.
inline constexpr int kBitmapMaximumScale = 4;
// An image a model file carries (GLB base colour and normal maps) is authored at
// up to 2048 x 2048; like upscaled art it is staged in bands of rows.
inline constexpr int kModelTextureMaximumWidth = 2048;
inline constexpr int kModelTextureMaximumHeight = 2048;
static_assert(kModelTextureMaximumWidth <= kBitmapMaximumWidth * kBitmapMaximumScale &&
              kModelTextureMaximumHeight <= kBitmapMaximumHeight * kBitmapMaximumScale);
} // namespace Render::Sprites
