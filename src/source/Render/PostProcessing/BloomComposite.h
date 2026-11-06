#pragma once

#include <cstddef>

// The arithmetic of the post-processing composite (post_composite.frag.hlsl and the bloom chain),
// kept free of the GPU so it can be checked on its own.
namespace Render::PostProcessing
{

// The summed weight of `levelCount` bloom levels, each added at `levelFalloff` times the weight of
// the next larger one (1, f, f^2, ...). The composite divides by it, so the bloom strength is the
// glow's brightness whatever the falloff.
[[nodiscard]] float BloomWeightSum(std::size_t levelCount, float levelFalloff);

// The grading strength the composite gets: 0 (ungraded) without a table or while a diagnostic
// view is shown, the look profile's strength otherwise.
[[nodiscard]] float CompositeGradingStrength(bool hasTable, bool diagnosticView, float strength);

// CPU mirror of the composite's grading blend: the colour channel between its ungraded (strength
// 0) and fully graded (strength 1) value. The shader is the implementation.
[[nodiscard]] float BlendGrading(float ungraded, float graded, float strength);

} // namespace Render::PostProcessing
