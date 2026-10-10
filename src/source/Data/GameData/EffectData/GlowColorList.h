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

private:
    std::vector<GlowColor> m_colors;
};
} // namespace Data::Effects

#define g_GlowColors Data::Effects::GlowColorList::GetInstance()
