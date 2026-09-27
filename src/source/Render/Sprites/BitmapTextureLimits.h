#pragma once

namespace Render::Sprites
{
// Whole-image RGBA uploads require a fixed staging allocation sized from these
// decoder limits. Raising either limit also raises the per-slot GPU budget.
inline constexpr int kBitmapMaximumWidth = 1024;
inline constexpr int kBitmapMaximumHeight = 1024;
} // namespace Render::Sprites
