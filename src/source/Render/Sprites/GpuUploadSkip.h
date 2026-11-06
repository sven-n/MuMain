#pragma once

namespace Render::Sprites
{
// For test binaries, which have no GPU device: while uploads are skipped, an
// image still loads, gets its bitmap number and is found by its file name,
// but has no texture. The game never skips them.
void SetGpuUploadsSkipped(bool skipped);
} // namespace Render::Sprites
