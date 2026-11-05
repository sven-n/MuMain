#pragma once

#include <RmlUi/Core/Types.h>

namespace UI::MapName
{
struct MapNameRmlModel
{
    // Placement and decoration belong to the theme; the name is plain UTF-8 text.
    float alpha = 1.f;
    Rml::String mapName;
    Rml::String language;     // the game's locale ("ja"): a theme can draw a language in its own face
    bool icon = false;        // the map's icon between the swirls
    Rml::String iconSource;   // Data/Interface/MapNameOrnament/<image>
    Rml::String iconStyle;    // where the icon sits on the banner
    bool strife = false;      // a Gens battle map: the strife banner above the name
    Rml::String strifeSource; // MapNameAddStrife
};
} // namespace UI::MapName
