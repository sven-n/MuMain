#pragma once

#include "Render/Renderer/MuRenderer.h"

#include <optional>
#include <string_view>

// Edge smoothing (FXAA): a full-screen pass over the processed 3D scene, after the composite and
// before the UI, that smooths the edges multisampling misses: highlights, rim light and texture
// detail inside a surface. It is part of the Enhanced look, beside multisampling, and needs the
// post-processed frame even when bloom and colour grading are switched off.
namespace Render::PostProcessing
{

// How much single bright or dark pixels are softened besides the edges (0 to 1): FXAA's own default,
// which keeps fine texture detail.
inline constexpr float EdgeSmoothingSubpixelStrength = 0.75f;

// Whether a frame takes the post-processed path: for bloom and colour grading, or for edge smoothing
// alone.
[[nodiscard]] bool UsesPostProcessedFrame(bool postProcessing, bool edgeSmoothing);

// The look the post-processed frame is composited with. With post-processing switched off and only
// edge smoothing on, the composite changes nothing: no bloom, no grading and no ambient occlusion.
[[nodiscard]] mu::PostProcessingLook CompositedLook(bool postProcessing, const mu::PostProcessingLook& look);

// The argument of `$fxaa`: "on" or "off", in any case and with surrounding spaces ignored. nullopt
// for anything else.
[[nodiscard]] std::optional<bool> ParseEdgeSmoothingCommand(std::wstring_view argument);

} // namespace Render::PostProcessing
