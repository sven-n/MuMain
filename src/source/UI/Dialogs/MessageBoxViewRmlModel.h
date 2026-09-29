#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A text line of a message box (RenderText() at its left and top, never shrunk).
struct MessageBoxViewLineEntry
{
    Rml::String text;
    float left = 0.f; // reference px in the box
    float top = 0.f;
    bool bold = false;
    Rml::String color;
    float textPx = 0.f; // physical px; 0: the font's native size (text_px / bold_text_px)
};

// A message box button (CMessageBoxButton): its art stretched to its box, its label at the
// original's whole-unit centre.
struct MessageBoxViewButtonEntry
{
    Rml::String label;
    int index = 0;
    float left = 0.f; // reference px in the box
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
    float labelLeft = 0.f; // reference px in the button
    float labelTop = 0.f;
    bool enabled = true;
    bool okArt = false; // newui_button_ok's lettered art, no label
};

// A line of a list inside the box (CUIUnmixgemList): its 13 px row box, filled when selected.
struct MessageBoxViewListRowEntry
{
    Rml::String text;
    float top = 0.f; // reference px in the box
    bool selected = false;
};

// One strip of the message box's frame: "middle" or "divider".
struct MessageBoxViewStripEntry
{
    Rml::String kind;
};

struct MessageBoxViewRmlModel
{
    // The message box manager's layout -- UI::Scaling::GetActiveTransform() while it runs.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;     // native normal text size in physical px
    float boldTextPx = 0.f; // native bold text size in physical px

    int middleCount = 0;           // 15-unit middle strips between the 67-unit top and 50-unit bottom
    float backHeight = 0.f;        // the newui_msgbox_back fill from y 2, 222 wide
    // The frame's strips in order, each saying only what it is; the theme stacks them and owns
    // every height. "middle" is a plain strip, "divider" the rule between two sections.
    std::vector<MessageBoxViewStripEntry> strips;
    std::vector<float> separators; // newui_separate_line (205 x 2) tops, 13 from the left
    std::vector<MessageBoxViewLineEntry> lines;
    // CProgressMsgBox's bar: newui_Bar_switch01 (160 x 18) centred at progressTop, the
    // newui_Bar_switch02 fill stretched to progressWidth (0 .. 150) inside it.
    bool progressShown = false;
    float progressTop = 0.f;
    float progressWidth = 0.f;
    std::vector<MessageBoxViewButtonEntry> buttons;

    // CGemIntegrationDisjointMsgBox's list (CUIUnmixgemList): its box, rows and old-style
    // (win_scrollbar) scroll bar, reference px in the box, drawn over everything else.
    bool listShown = false;
    float listLeft = 0.f, listTop = 0.f, listWidth = 0.f, listHeight = 0.f;
    std::vector<MessageBoxViewListRowEntry> listRows;
    bool listUpPressed = false, listDownPressed = false;
    float listTrackTop = 0.f, listTrackHeight = 0.f;
    float listThumbTop = 0.f, listThumbHeight = 0.f, listThumbBottomTop = 0.f;
};
} // namespace mu::ui::window
