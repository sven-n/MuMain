///////////////////////////////////////////////////////////////////////////////
// Random.h
//
// Thread-safe random number generation module.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <random>

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

/// The game's legacy sequence: rand() as the Windows CRT gives it, the same values on every platform, one sequence per
/// thread starting at seed 1. The game draws from it where it called rand().
std::int32_t Rand();
/// srand() for the legacy sequence of the calling thread.
void SeedRand(std::uint32_t seed);
/// The calling thread's legacy state, to tell whether something drew from the sequence.
std::uint32_t RandState();

/// Restarts the calling thread's sequence from a fixed seed, so that a test
/// can repeat it (the effect recorder). The game does not call it.
void Seed(std::uint32_t seed);

/// A copy of the calling thread's engine, to tell whether something drew from its sequence.
std::mt19937 Snapshot();

/// Starts the frame the draws' random values come from; the last step of the update.
void NextDrawFrame();

/// A draw's random value in [0, 32767], as rand() gives: fixed for the frame, the key and the salt, so every pass of
/// a frame draws the same, and no sequence the update draws from moves.
std::int32_t DrawRand(const void* key, std::uint32_t salt);

/// FpsCheck for a draw, from DrawRand: the same in every pass of a frame.
bool DrawFpsCheck(std::int32_t referenceFrames, double animationFactor, const void* key, std::uint32_t salt);

} // namespace Random
