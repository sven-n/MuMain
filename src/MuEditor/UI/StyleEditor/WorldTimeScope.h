#pragma once

#ifdef _EDITOR

#include "Render/Textures/ZzzOpenglUtil.h"

#include <optional>

namespace MuEditor::Looks
{
// The clock of the game while it lives: the Style Editor preview's stopped time, in milliseconds of WorldTime;
// without one the clock runs on.
class WorldTimeScope
{
public:
    explicit WorldTimeScope(std::optional<double> milliseconds) : m_previous(WorldTime), m_set(milliseconds.has_value())
    {
        if (m_set)
        {
            WorldTime = *milliseconds;
        }
    }
    ~WorldTimeScope()
    {
        if (m_set)
        {
            WorldTime = m_previous;
        }
    }
    WorldTimeScope(const WorldTimeScope&) = delete;
    WorldTimeScope& operator=(const WorldTimeScope&) = delete;

private:
    double m_previous;
    bool m_set;
};
} // namespace MuEditor::Looks

#endif // _EDITOR
