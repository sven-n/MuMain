///////////////////////////////////////////////////////////////////////////////
// Random.cpp
//
// Implementation of thread-safe random number generation.
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Random.h"

#include <algorithm>
#include <random>

namespace Random
{
    namespace
    {
        /// Thread-local Mersenne Twister engine with high-quality seeding.
        /// Each thread gets its own independent RNG state for thread safety.
        /// Seeded using multiple calls to std::random_device for better entropy.
        std::mt19937& GetThreadEngine()
        {
            thread_local static std::mt19937 engine{
                []()
                {
                    std::seed_seq seed{
                        std::random_device{}(),
                        std::random_device{}(),
                        std::random_device{}(),
                        std::random_device{}()
                    };
                    return std::mt19937{seed};
                }()
            };
            return engine;
        }

        /// Adapter that clamps the RNG output to 31 bits to avoid
        /// runtime narrowing checks when casting to signed 32-bit types.
        struct Engine31
        {
            using result_type = std::uint32_t;

            static constexpr result_type min()
            {
                return 0;
            }

            static constexpr result_type max()
            {
                return 0x7FFFFFFFu;
            }

            result_type operator()()
            {
                return GetThreadEngine()() & max();
            }
        };

    } // anonymous namespace

    std::int32_t RangeInt(std::int32_t minInclusive, std::int32_t maxInclusive)
    {
        if (minInclusive >= maxInclusive)
        {
            return minInclusive;
        }

        thread_local static std::uniform_int_distribution<std::int32_t> dist;
        thread_local static Engine31 engine31;
        using param_type = std::uniform_int_distribution<std::int32_t>::param_type;
        return dist(engine31, param_type(minInclusive, maxInclusive));
    }

    float RangeFloat(float minInclusive, float maxInclusive)
    {
        if (minInclusive >= maxInclusive)
        {
            return minInclusive;
        }

        thread_local static std::uniform_real_distribution<float> dist;
        using param_type = std::uniform_real_distribution<float>::param_type;
        return dist(GetThreadEngine(), param_type(minInclusive, maxInclusive));
    }

    float Unit()
    {
        thread_local static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(GetThreadEngine());
    }

    double UnitDouble()
    {
        thread_local static std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(GetThreadEngine());
    }

    bool FpsCheck(std::int32_t referenceFrames, double animationFactor)
    {
        if (referenceFrames <= 0)
        {
            return false;
        }

        const double clampedFactor = std::min<double>(1.0, std::max<double>(0.0, animationFactor));
        
        const double randomValue = UnitDouble();
        
        const double chance = (referenceFrames == 1)
            ? clampedFactor
            : (1.0 / static_cast<double>(referenceFrames)) * clampedFactor;

        return randomValue <= chance;
    }

} // namespace Random
