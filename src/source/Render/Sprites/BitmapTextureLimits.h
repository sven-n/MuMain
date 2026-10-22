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
} // namespace Render::Sprites
