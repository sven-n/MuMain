#pragma once

#include <RmlUi/Core/Types.h>

namespace UI::MapName
{
struct MapNameRmlModel
{
    // Physical px, like the original: the name image's top-left (the strife banner sits right above
    // it) and the fade alpha.
    float left = 0.f;
    float top = 0.f;
    float alpha = 1.f;
    Rml::String imageSource;  // the map's name image as CGlobalBitmap loaded it
    bool strife = false;      // a Gens battle map: the strife banner above the name
    Rml::String strifeSource; // MapNameAddStrife
};
} // namespace UI::MapName
