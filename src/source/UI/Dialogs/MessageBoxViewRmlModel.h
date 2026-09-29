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
};

struct MessageBoxViewRmlModel
{
    // The message box manager's layout -- UI::Scaling::GetActiveTransform() while it runs.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;     // native normal text size in physical px
    float boldTextPx = 0.f; // native bold text size in physical px

    int middleCount = 0;      // 15-unit middle strips between the 67-unit top and 50-unit bottom
    float backHeight = 0.f;   // the newui_msgbox_back fill from y 2, 222 wide
    std::vector<int> middles; // 0 .. middleCount - 1, for data-for
    std::vector<MessageBoxViewLineEntry> lines;
    // CProgressMsgBox's bar: newui_Bar_switch01 (160 x 18) centred at progressTop, the
    // newui_Bar_switch02 fill stretched to progressWidth (0 .. 150) inside it.
    bool progressShown = false;
    float progressTop = 0.f;
    float progressWidth = 0.f;
    std::vector<MessageBoxViewButtonEntry> buttons;
};
} // namespace mu::ui::window
