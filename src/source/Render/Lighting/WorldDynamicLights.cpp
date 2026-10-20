#include "Render/Lighting/WorldDynamicLights.h"

#include "Render/Lighting/DynamicLightList.h"

namespace Render::Lighting
{

void WorldDynamicLights::Clear()
{
    m_pinned = false;
}

void WorldDynamicLights::Pin(DynamicLightList& lights, const float (&reference)[3])
{
    if (m_pinned)
    {
        return;
    }
    m_selection = SelectDynamicLights(lights.Lights(), reference);
    lights.Mark();
    m_pinned = true;
}

WorldDynamicLights& FrameWorldDynamicLights()
{
    static WorldDynamicLights lights;
    return lights;
}

} // namespace Render::Lighting
