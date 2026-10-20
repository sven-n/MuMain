// WorldDynamicLights.h: the frame's point lights the ground applies per pixel.
#pragma once

#include "Render/Lighting/DynamicLightSelection.h"

namespace Render::Lighting
{

class DynamicLightList;

// The ground's vertex light leaves out exactly the lights pinned here and the ground's shader adds
// exactly these back per pixel, so they are chosen once per frame, before the ground is drawn, and
// kept even when lights added later would rank higher. Cleared with the frame's light list.
class WorldDynamicLights
{
public:
    void Clear();
    // Pins the selection SelectDynamicLights makes of `lights` at `reference`, all of them world lights
    // (worldLightCount), and marks the list,
    // so the lights added after it are told apart. A second call in the frame keeps the first pin.
    void Pin(DynamicLightList& lights, const float (&reference)[3]);

    [[nodiscard]] bool IsPinned() const
    {
        return m_pinned;
    }
    [[nodiscard]] const DynamicLightSelection& Selection() const
    {
        return m_selection;
    }

private:
    DynamicLightSelection m_selection{};
    bool m_pinned = false;
};

// The pin the game's ground and its point-light upload share.
[[nodiscard]] WorldDynamicLights& FrameWorldDynamicLights();

} // namespace Render::Lighting
