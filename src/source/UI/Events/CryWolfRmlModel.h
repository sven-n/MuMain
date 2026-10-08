#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One image of the HUD that sits where the theme puts it: an altar of the five, or one of the
// nine experience digits. An altar with no contract state shows nothing, and the five are always
// all five so a theme can count them.
struct CryWolfImageEntry
{
    bool shown = true;
    Rml::String src;
    Rml::String rect;

    bool operator==(const CryWolfImageEntry&) const = default;
};

// One line of the ready-state notice, drawn in the bold font on a translucent black box. The
// first line is the heading, which the original coloured apart; the theme places all four.
struct CryWolfNoticeEntry
{
    Rml::String text;
    bool heading = false;

    bool operator==(const CryWolfNoticeEntry&) const = default;
};

struct CryWolfRmlModel
{
    // The HUD board (LayoutMode::HudCenter while CManager runs the window): the 640x480 frame's
    // offset in physical px and its one scale, cancelled by the counter-scaled text leaves.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
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
    std::vector<CryWolfImageEntry> expDigits;

    // The battle HUD (ready and start states).
    bool hudVisible = false;
    std::vector<CryWolfImageEntry> altars;
    Rml::String darkElfIconSrc;
    Rml::String darkElfText;
    bool balgassVisible = false;
    Rml::String balgassText;
    float balgassBarWidth = 0.f;
    Rml::String balgassBarRect;
    // The clock's FontTest cells (UI::RmlBridge::DigitCells), each with a leading zero below ten.
    bool minutePad = true;
    std::vector<Rml::String> minuteDigits;
    bool secondPad = true;
    std::vector<Rml::String> secondDigits;
    // Balgass on the field, which the original reddened the clock for.
    bool timerUrgent = false;
    float statueBarLeft = 0.f;
    float statueBarWidth = 0.f;
    Rml::String statueBarRect;
    std::vector<CryWolfNoticeEntry> notices;
};
} // namespace mu::ui::window
