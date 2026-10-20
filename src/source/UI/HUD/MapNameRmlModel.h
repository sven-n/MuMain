#pragma once

#include <RmlUi/Core/Types.h>

namespace UI::MapName
{
struct MapNameRmlModel
{
    // Placement and size belong to the theme; C++ supplies the fade and current artwork.
    float alpha = 1.f;
    Rml::String imageSource;  // the map's name image as CGlobalBitmap loaded it
    bool strife = false;      // a Gens battle map: the strife banner above the name
    Rml::String strifeSource; // MapNameAddStrife
};
} // namespace UI::MapName
