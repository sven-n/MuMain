#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One node of the master skill tree. Position and sprite come from
// UI::Skills::MasterTree; `usable` is the meaning behind native's lit/grey icon and white/grey
// level text, so each theme owns the look.
struct MasterLevelNodeEntry
{
    int id = 0;       // the node's tree index (the key of CMasterLevel::map_masterData)
    float left = 0.f; // node box, reference px
    float top = 0.f;
    Rml::String icon; // decorator value, e.g. "image(master-icon-lit-137)"
    bool usable = false;
    int arrow = 0; // _MASTER_SKILLTREE_DATA::ArrowDirection, 0 = none
    Rml::String levelText;
};

struct MasterLevelRmlModel
{
    // The Hud layout's W/640 x H/480 stretch (UI::Scaling::GetActiveTransform() while
    // CManager runs this window), and its inverse for the counter-scaled text leaves.
    float scaleX = 1.f, scaleY = 1.f;
    float inverseScaleX = 1.f, inverseScaleY = 1.f;
    float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)

    Rml::String classNameText;
    Rml::String masterLevelText;
    Rml::String levelPointText;
    Rml::String experienceText; // empty while the next level's experience is unknown
    Rml::String columnText0;
    Rml::String columnText1;
    Rml::String columnText2;

    std::vector<MasterLevelNodeEntry> nodes;
};
} // namespace mu::ui::window
