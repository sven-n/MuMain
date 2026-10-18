// DynamicLightList.h: the dynamic lights added during the current frame.
#pragma once

#include "Render/Lighting/DynamicLight.h"

#include <array>
#include <cstddef>
#include <span>

namespace Render::Lighting
{

// Fixed-capacity list, cleared once per frame together with the terrain light (InitTerrainLight).
// A frame that adds more lights than it holds keeps the first ones and counts the rest, except that
// lights added after MarkDisplaceable() (lights that only linger from earlier frames, weakest last)
// give way to later lights added after EndDisplaceable(): a full list then drops its last displaceable
// light instead. A displaceable light that finds the list full is counted, like any other.
class DynamicLightList
{
public:
    static constexpr std::size_t kCapacity = 1024;

    void Clear();
    void Add(const DynamicLight& light);
    // The lights added until EndDisplaceable() may give way to later ones.
    void MarkDisplaceable();
    // Ends the lingering interval; later additions are called lights, never evictable.
    void EndDisplaceable();

    [[nodiscard]] std::span<const DynamicLight> Lights() const;
    // Lights added this frame that did not fit.
    [[nodiscard]] std::size_t Overflow() const
    {
        return m_overflow;
    }

private:
    std::array<DynamicLight, kCapacity> m_lights{};
    std::size_t m_count = 0;
    std::size_t m_overflow = 0;
    // Displaceable lights are [m_displaceableBegin, m_displaceableEnd).
    std::size_t m_displaceableBegin = kCapacity;
    std::size_t m_displaceableEnd = kCapacity;
    bool m_marking = false;
};

// The list the game's terrain light calls record into.
[[nodiscard]] DynamicLightList& FrameDynamicLights();

} // namespace Render::Lighting
