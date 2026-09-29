#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One image of the Crywolf HUD whose file or cell changes: `src` relative to crywolf.rml,
// `rect` its texel cell, `left`/`top` reference px on the 640x480 screen.
struct CryWolfSpriteEntry
{
    float left = 0.f;
    float top = 0.f;
    Rml::String src;
    Rml::String rect;
};

// One line of the ready-state notice, drawn in the bold font on a translucent black box.
struct CryWolfNoticeEntry
{
    float top = 0.f; // reference px
    Rml::String text;
    Rml::String color;
};

struct CryWolfRmlModel
{
    // The Hud layout's W/640 x H/480 stretch (UI::Scaling::GetActiveTransform() while CManager
    // runs the window) and its inverse for the counter-scaled text leaves.
    float scaleX = 1.f, scaleY = 1.f;
    float inverseScaleX = 1.f, inverseScaleY = 1.f;
    float normalTextPx = 0.f; // native normal text size in physical px
    float boldTextPx = 0.f;   // native bold text size in physical px
    float boldLinePx = 0.f;   // the bold font's line box in physical px (the notice's background)

    // The end-of-battle result: the success or failure banner sliding in, holding and sliding
    // out, the rank table, the rank label sliding in and then the rank letter and the experience.
    bool resultVisible = false;
    float bannerLeft = 0.f;
    Rml::String bannerSrc;
    float bannerOpacity = 1.f; // the banner's fade; the theme owns its colour
    float rankLabelLeft = 0.f;
    bool rankDetailsVisible = false;
    Rml::String rankLetterSrc; // empty for a rank the original had no letter for
    std::vector<CryWolfSpriteEntry> expDigits;

    // The battle HUD (ready and start states).
    bool hudVisible = false;
    std::vector<CryWolfSpriteEntry> altars;
    Rml::String darkElfIconSrc;
    Rml::String darkElfText;
    bool balgassVisible = false;
    Rml::String balgassText;
    float balgassBarWidth = 0.f;
    Rml::String balgassBarRect;
    std::vector<CryWolfSpriteEntry> timerDigits;
    Rml::String timerColor;
    float statueBarLeft = 0.f;
    float statueBarWidth = 0.f;
    Rml::String statueBarRect;
    std::vector<CryWolfNoticeEntry> notices;
};
} // namespace mu::ui::window
