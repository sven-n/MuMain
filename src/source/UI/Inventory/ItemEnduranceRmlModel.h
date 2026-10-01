#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace UI::ItemEndurance
{
// One pet / summon HP frame of the left column: its top in reference px (the column's transform),
// the drawn part of the health bar and the name, centred in physical px.
struct PetFrameEntry
{
    float top = 0.f;
    float barWidth = 0.f;
    Rml::String name;
    float nameCentreX = 0.f; // physical px
    float nameTop = 0.f;     // physical px

    bool operator==(const PetFrameEntry& other) const
    {
        return top == other.top && barWidth == other.barWidth && name == other.name &&
               nameCentreX == other.nameCentreX && nameTop == other.nameTop;
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
    // Left column (pet / summon HP frames): the screen overlay transform, reference px inside.
    float leftX = 0.f;
    float leftY = 0.f;
    float leftScaleX = 1.f;
    float leftScaleY = 1.f;
    // Right-hand durability icons: the dock-right transform, reference px inside.
    float rightX = 0.f;
    float rightY = 0.f;
    // Where the icon column starts: right-anchored to the live screen width, so it is the window's
    // to decide; the pack inside it is the theme's.
    float iconsLeft = 0.f;
    float iconsTop = 0.f;
    float rightScaleX = 1.f;
    float rightScaleY = 1.f;

    // Native text sizes (physical px): the normal font of the left column, the bold tooltip.
    float textPx = 0.f;
    float lineHeightPx = 0.f;
    float boldTextPx = 0.f;
    float boldLineHeightPx = 0.f;

    // The elf's arrow / bolt count line, physical px; empty when none is shown.
    Rml::String arrows;
    float arrowsLeft = 0.f;
    float arrowsTop = 0.f;

    std::vector<PetFrameEntry> pets;
    std::vector<DurabilityIconEntry> icons;

    // The hovered icon's tooltip, centred on tooltipCentreX, physical px; empty when none.
    Rml::String tooltip;
    Rml::String tooltipBand;
    float tooltipCentreX = 0.f;
    float tooltipTop = 0.f;
};
} // namespace UI::ItemEndurance
