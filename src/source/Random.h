///////////////////////////////////////////////////////////////////////////////
// Random.h
//
// Thread-safe random number generation module.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>

namespace Random
{
    /// Returns a random integer in the range [minInclusive, maxInclusive].
    /// Thread-safe. Both bounds are inclusive.
    /// If min >= max, returns minInclusive.
    std::int32_t RangeInt(std::int32_t minInclusive, std::int32_t maxInclusive);

    /// Returns a random float in the range [minInclusive, maxInclusive].
    /// Thread-safe. Both bounds are inclusive.
    /// If min >= max, returns minInclusive.
    float RangeFloat(float minInclusive, float maxInclusive);

    /// Returns a random float in the range [0.0, 1.0].
    /// Thread-safe. Useful for probability checks.
    float Unit();

    /// Returns a random double in the range [0.0, 1.0].
    /// Thread-safe. Higher precision than Unit().
    double UnitDouble();

    /// FPS-aware probability check.
    /// Returns true with a probability adjusted for current frame rate.
    /// 
    /// @param referenceFrames - The reference frame interval (e.g., 10 means "once every 10 frames at reference FPS")
    /// @param animationFactor - Current FPS scaling factor (typically FPS_ANIMATION_FACTOR from the engine)
    /// 
    /// At reference FPS (animationFactor = 1.0):
    ///   - referenceFrames=1 -> 100% chance per frame
    ///   - referenceFrames=10 -> 10% chance per frame
    ///   - referenceFrames=60 -> ~1.67% chance per frame
    /// 
    /// At lower FPS (animationFactor < 1.0), probability is scaled proportionally
    /// to maintain similar event frequency over real time.
    /// 
    /// Thread-safe.
    bool FpsCheck(std::int32_t referenceFrames, double animationFactor);

    /// Simplified FPS check with default animation factor of 1.0.
    /// Useful for non-FPS-dependent random events.
    /// Thread-safe.
    inline bool FpsCheck(std::int32_t referenceFrames)
    {
        return FpsCheck(referenceFrames, 1.0);
    }

} // namespace Random

