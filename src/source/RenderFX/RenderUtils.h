#pragma once
// RenderUtils.h — Shared render utility helpers for the RenderFX module.
//
// Story 4.2.4 (code-review-finalize, H-1 fix): Extracted PackABGR from per-file
// static inline copies in ZzzEffectJoint.cpp, ZzzBMD.cpp, and
// test_traileffects_migration.cpp into a single shared definition to eliminate
// silent divergence risk. All three sites were previously guarded by
// "KEEP IN SYNC WITH" comments; this header removes that burden.
//
// Included by: ZzzEffectJoint.cpp, ZzzBMD.cpp, tests/render/test_traileffects_migration.cpp
// ---------------------------------------------------------------------------

#include <cstdint>

namespace mu
{

// PackABGR: Pack float RGBA channels into a 32-bit ABGR value.
// Layout: A=bits31-24, B=bits23-16, G=bits15-8, R=bits7-0
// Channels are clamped to [0.0, 1.0] to handle overbright luminosity values
// (e.g., VectorScale with Luminosity > 1.0 would otherwise wrap the byte).
[[nodiscard]] inline std::uint32_t PackABGR(float r, float g, float b, float a) noexcept
{
    auto clamp01 = [](float v) noexcept -> float { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); };
    return (static_cast<std::uint32_t>(clamp01(a) * 255.f) << 24) |
           (static_cast<std::uint32_t>(clamp01(b) * 255.f) << 16) |
           (static_cast<std::uint32_t>(clamp01(g) * 255.f) << 8) | static_cast<std::uint32_t>(clamp01(r) * 255.f);
}

} // namespace mu
