#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace UI::ItemEndurance
{
// One pet / summon HP frame of the left column: the drawn part of the health bar and the name.
struct PetFrameEntry
{
    float barWidth = 0.f;
    Rml::String name;

    bool operator==(const PetFrameEntry& other) const
    {
        return barWidth == other.barWidth && name == other.name;
    }
};

// One durability warning of the right-hand icons: which cell of the column it occupies (two per
// column, filling downwards then leftwards, which the theme lays out), the icon (an empty image
// for the left ring when the right ring already drew the shared ring icon) and which part of it
// the tint covers -- the whole icon, or one half each when both rings warn at once.
struct DurabilityIconEntry
{
    int cell = 0;
    Rml::String image;    // "boots", "cap", ... ; empty = tint only
    Rml::String tintHalf; // "", "left", "right"
    Rml::String band;     // "half", "third", "fifth", "zero": at most 50 / 30 / 20 %, or worn out

    bool operator==(const DurabilityIconEntry& other) const
    {
        return cell == other.cell && image == other.image && tintHalf == other.tintHalf && band == other.band;
    }
};

struct ItemEnduranceRmlModel
{

    // Native text sizes (physical px): the normal font of the left column, the bold tooltip.
    float textPx = 0.f;
    float lineHeightPx = 0.f;
    float boldTextPx = 0.f;
    float boldLineHeightPx = 0.f;

    // The elf's arrow / bolt count line; empty when none is shown.
    Rml::String arrows;

    std::vector<PetFrameEntry> pets;
    std::vector<DurabilityIconEntry> icons;

    // The hovered icon's tooltip, centred on tooltipCentreX, window pixels; empty when none.
    Rml::String tooltip;
    Rml::String tooltipBand;
    float tooltipCentreX = 0.f;
    float tooltipTop = 0.f;
};
} // namespace UI::ItemEndurance
