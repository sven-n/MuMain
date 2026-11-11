///////////////////////////////////////////////////////////////////////////////
// Random.cpp
//
// Implementation of thread-safe random number generation.
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core/Utilities/Random.h"

#include "Core/Utilities/DrawSpawnCheck.h"

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

namespace Random
{
namespace
{
/// Thread-local Mersenne Twister engine with high-quality seeding.
/// Each thread gets its own independent RNG state for thread safety.
/// Seeded using multiple calls to std::random_device for better entropy.
std::mt19937& GetThreadEngine()
{
    thread_local static std::mt19937 engine{[]()
                                            {
                                                std::seed_seq seed{std::random_device{}(), std::random_device{}(),
                                                                   std::random_device{}(), std::random_device{}()};
                                                return std::mt19937{seed};
                                            }()};
    return engine;
}

std::uint64_t s_drawFrame = 0;

thread_local std::uint32_t s_randState = 1;

// splitmix64's finalizer: every input bit reaches every output bit.
std::uint64_t Mix(std::uint64_t x)
{
    x += 0x9E3779B97F4A7C15ull;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
    return x ^ (x >> 31);
}

double EngineUnitDouble()
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(GetThreadEngine());
}

double ChanceOf(std::int32_t referenceFrames, double animationFactor)
{
    const double clampedFactor = std::min<double>(1.0, std::max<double>(0.0, animationFactor));
    return (referenceFrames == 1) ? clampedFactor : (1.0 / static_cast<double>(referenceFrames)) * clampedFactor;
}

} // anonymous namespace

// Each call that advances or reseeds a gameplay sequence tells the draw guard first: the draws take theirs from
// DrawRand. Reading the state (RandState, Snapshot) changes nothing and tells nothing.

std::int32_t Rand()
{
    Diagnostics::DrawSpawnCheck::Note("legacy random draw");
    // The Windows CRT's linear congruential generator.
    s_randState = s_randState * 214013u + 2531011u;
    return static_cast<std::int32_t>((s_randState >> 16) & 0x7FFF);
}

void SeedRand(std::uint32_t seed)
{
    Diagnostics::DrawSpawnCheck::Note("legacy random reseed");
    s_randState = seed;
}

std::uint32_t RandState()
{
    return s_randState;
}

// The distributions are made per call: they are cheap and keep no state, so
// the engine alone decides the sequence and Seed repeats it.

std::int32_t RangeInt(std::int32_t minInclusive, std::int32_t maxInclusive)
{
    if (minInclusive >= maxInclusive)
    {
        return minInclusive;
    }

    Diagnostics::DrawSpawnCheck::Note("random engine draw");
    std::uniform_int_distribution<std::int32_t> dist(minInclusive, maxInclusive);
    return dist(GetThreadEngine());
}

float RangeFloat(float minInclusive, float maxInclusive)
{
    if (minInclusive >= maxInclusive)
    {
        return minInclusive;
    }

    Diagnostics::DrawSpawnCheck::Note("random engine draw");
    std::uniform_real_distribution<float> dist(minInclusive, maxInclusive);
    return dist(GetThreadEngine());
}

float Unit()
{
    Diagnostics::DrawSpawnCheck::Note("random engine draw");
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(GetThreadEngine());
}

double UnitDouble()
{
    Diagnostics::DrawSpawnCheck::Note("random engine draw");
    return EngineUnitDouble();
}

bool FpsCheck(std::int32_t referenceFrames, double animationFactor)
{
    if (referenceFrames <= 0)
    {
        return false;
    }

    Diagnostics::DrawSpawnCheck::Note("random engine draw");
    const double randomValue = EngineUnitDouble();
    return randomValue <= ChanceOf(referenceFrames, animationFactor);
}

void Seed(std::uint32_t seed)
{
    Diagnostics::DrawSpawnCheck::Note("random engine reseed");
    GetThreadEngine().seed(seed);
}

std::mt19937 Snapshot()
{
    return GetThreadEngine();
}

void NextDrawFrame()
{
    ++s_drawFrame;
}

namespace
{
// The copies drawn as other objects on this thread, innermost last.
thread_local std::vector<std::pair<const void*, const void*>> s_drawnAs;
} // namespace

DrawnAs::DrawnAs(const void* copy, const void* source)
{
    s_drawnAs.emplace_back(copy, DrawKey(source));
}

DrawnAs::~DrawnAs()
{
    s_drawnAs.pop_back();
}

const void* DrawKey(const void* key)
{
    for (auto it = s_drawnAs.rbegin(); it != s_drawnAs.rend(); ++it)
        if (it->first == key)
            return it->second;
    return key;
}

std::int32_t DrawRand(const void* key, std::uint32_t salt)
{
    const auto at = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(DrawKey(key)));
    return static_cast<std::int32_t>(Mix(Mix(s_drawFrame ^ (static_cast<std::uint64_t>(salt) << 32)) ^ at) & 0x7fff);
}

bool DrawFpsCheck(std::int32_t referenceFrames, double animationFactor, const void* key, std::uint32_t salt)
{
    if (referenceFrames <= 0)
    {
        return false;
    }
    return DrawRand(key, salt) / 32767.0 <= ChanceOf(referenceFrames, animationFactor);
}

} // namespace Random
