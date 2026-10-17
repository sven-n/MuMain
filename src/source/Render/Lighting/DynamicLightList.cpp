#include "Render/Lighting/DynamicLightList.h"

#include <algorithm>

namespace Render::Lighting
{

void DynamicLightList::Clear()
{
    m_count = 0;
    m_overflow = 0;
    m_displaceableBegin = kCapacity;
    m_displaceableEnd = kCapacity;
    m_marking = false;
}

void DynamicLightList::MarkDisplaceable()
{
    m_marking = true;
    m_displaceableBegin = m_count;
    m_displaceableEnd = m_count;
}

void DynamicLightList::EndDisplaceable()
{
    m_marking = false;
}

void DynamicLightList::Add(const DynamicLight& light)
{
    if (m_count == m_lights.size())
    {
        // Displaceable lights arrive strongest first: one that finds the list full is the weakest of
        // them and is the one that does not fit.
        if (m_marking || m_displaceableEnd == m_displaceableBegin || m_displaceableEnd > m_count)
        {
            ++m_overflow;
            return;
        }
        // Drop the last (weakest) displaceable light, keeping the order of the others.
        std::move(m_lights.begin() + static_cast<std::ptrdiff_t>(m_displaceableEnd),
                  m_lights.begin() + static_cast<std::ptrdiff_t>(m_count),
                  m_lights.begin() + static_cast<std::ptrdiff_t>(m_displaceableEnd) - 1);
        --m_displaceableEnd;
        --m_count;
        m_marking = false;
    }
    m_lights[m_count++] = light;
    if (m_marking)
    {
        m_displaceableEnd = m_count;
    }
}

std::span<const DynamicLight> DynamicLightList::Lights() const
{
    return {m_lights.data(), m_count};
}

DynamicLightList& FrameDynamicLights()
{
    static DynamicLightList lights;
    return lights;
}

} // namespace Render::Lighting
