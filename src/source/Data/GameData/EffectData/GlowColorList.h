#pragma once

#include "Data/GameData/EffectData/GlowColors.h"

#include <span>
#include <string_view>
#include <vector>

namespace Data::Effects
{
// The glow colors by name. Built once at startup, before the item models
// are checked.
class GlowColorList
{
public:
    static GlowColorList& GetInstance();

    void Build(std::span<const GlowColor> colors);

    // Returns nullptr for names that are not in the list.
    const GlowColorValue* Find(std::string_view name) const;

    // All colors, in the order of the file (the item editor lists them).
    std::span<const GlowColor> GetColors() const
    {
        return m_colors;
    }

#ifdef _EDITOR
    // Adds the color after the others, or gives the one of its name its
    // value. For the item editor; Find's pointers do not stay valid.
    void Set(const GlowColor& color);
    void Remove(std::string_view name);
    // Counts the builds and changes, for what is shown from the list.
    int GetVersion() const
    {
        return m_version;
    }
#endif

private:
    std::vector<GlowColor> m_colors;
#ifdef _EDITOR
    int m_version = 0;
#endif
};
} // namespace Data::Effects

#define g_GlowColors Data::Effects::GlowColorList::GetInstance()
