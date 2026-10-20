#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A text of a message box line: its font, colour and size (a box the original shrank it to).
struct MessageBoxViewCellEntry
{
    Rml::String text;
    bool bold = false;
    Rml::String color;
    float textPx = 0.f; // physical px; 0: the font's native size (text_px / bold_text_px)

    bool operator==(const MessageBoxViewCellEntry&) const = default;
};

// A line of a message box: what it is (role; the theme spaces and places it by that and the box's
// kind), its font, and its text or a table row's columns.
struct MessageBoxViewLineEntry
{
    Rml::String role;
    bool bold = false;
    std::vector<MessageBoxViewCellEntry> cells;

    bool operator==(const MessageBoxViewLineEntry&) const = default;
};

// A message box button (CMessageBoxButton): its art, its label centred on it. The theme places it
// by the box's kind.
struct MessageBoxViewButtonEntry
{
    Rml::String label;
    int index = 0;
    bool enabled = true;
    bool okArt = false; // newui_button_ok's lettered art, no label

    bool operator==(const MessageBoxViewButtonEntry&) const = default;
};

// A selectable jewel bundle in the message box.
struct MessageBoxViewListRowEntry
{
    Rml::String text;
    int index = -1;
    bool selected = false;
    bool operator==(const MessageBoxViewListRowEntry&) const = default;
};

// One strip of the message box's frame: "middle" or "divider".
struct MessageBoxViewStripEntry
{
    Rml::String kind;
};

struct MessageBoxViewRmlModel
{
    // The box's kind, #panel's data-kind: the theme places the box, its lines and its buttons by it.
    Rml::String kind;
    float textPx = 0.f;     // native normal text size in physical px
    float boldTextPx = 0.f; // native bold text size in physical px
    float lineHeight = 0.f; // the native line heights, reference px
    float boldLineHeight = 0.f;

    int middleCount = 0;           // 15-unit middle strips between the 67-unit top and 50-unit bottom
    // The frame's strips in order, each saying only what it is; the theme stacks them and owns
    // every height. "middle" is a plain strip, "divider" the rule between two sections.
    std::vector<MessageBoxViewStripEntry> strips;
    std::vector<MessageBoxViewLineEntry> lines;
    // CProgressMsgBox's bar: newui_Bar_switch01 (160 x 18), the newui_Bar_switch02 fill stretched
    // to progressWidth (0 .. 150) inside it.
    bool progressShown = false;
    float progressWidth = 0.f;
    std::vector<MessageBoxViewButtonEntry> buttons;

    bool listShown = false;
    std::vector<MessageBoxViewListRowEntry> listRows;
};
} // namespace mu::ui::window
