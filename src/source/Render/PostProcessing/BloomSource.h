#pragma once

#include "Render/Renderer/MuRenderer.h"

namespace Render::PostProcessing
{

// True for draws that add light to what is behind them with the blend modes of
// MU's glow effects (skills, portals, torches): these, and only these, feed the
// bloom. Opaque, alpha-blended and darkening draws never do, and neither does a
// draw recorded inside a bloom exclusion (`excluded`): a model's chrome pass,
// which reflects light off the surface, and characters with their equipment,
// wings, pets and mounts, which keep their Classic look.
[[nodiscard]] bool IsBloomSource(bool blendEnabled, mu::BlendMode mode, bool excluded);

} // namespace Render::PostProcessing
