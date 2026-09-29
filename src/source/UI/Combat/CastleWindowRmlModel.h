#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One draw of the Senatus page, in the original's paint order: a sprite (the table edges, the
// castle map, the gate and statue icons, the separator line, the money strip), a
// RenderColorQuadARGB() box or a RenderText().
struct CastlePieceEntry
{
    enum Kind
    {
        KIND_IMAGE = 0,
        KIND_BOX = 1,
        KIND_TEXT = 2,
    };

    int kind = KIND_IMAGE;
    Rml::String sprite; // KIND_IMAGE: the rcss class naming the sprite
    float left = 0.f;   // reference px in the panel
    float top = 0.f;
    float width = 0.f; // KIND_TEXT: 0 = no box
    float height = 0.f;
    Rml::String color; // KIND_BOX / KIND_TEXT
    Rml::String text;
    float textPx = 0.f;
    int align = 0; // 0 left, 1 centred, 2 right edge at left + width
    bool bold = false;

    bool operator==(const CastlePieceEntry&) const = default;
};

// One of the four newui_guild_tab04 radio tabs (the left 40 x 22 of the sprite): the selected
// one on its second row, its label white, the others (181, 181, 181).
struct CastleTabEntry
{
    Rml::String label;
    float labelLeft = 0.f; // CRadioButton::Render(): whole-unit centre in the tab
    float textPx = 0.f;    // shrunk towards the tab's 40 px like the native RenderText()
    bool selected = false;

    bool operator==(const CastleTabEntry&) const = default;
};

// A 53 x 23 newui_btn_empty_very_small button (Buy, Repair, Improve, Apply, Withdraw): locked
// ones tinted (100, 100, 100) with a grey label, never hovered.
struct CastleButtonEntry
{
    Rml::String label;
    int id = 0; // CCastleWindow::SENATUS_BUTTON
    float left = 0.f;
    float top = 0.f;
    bool locked = false;

    bool operator==(const CastleButtonEntry&) const = default;
};

struct CastleWindowRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)
    float lineHeightPx = 0.f;
    float buttonLabelTop = 0.f; // CButton::Render(): 23 / 2 - h / 2 whole units
    float tabLabelTop = 0.f;    // CRadioButton::Render(): 22 / 2 - h / 2 whole units

    std::vector<CastleTabEntry> tabs;
    std::vector<CastlePieceEntry> pieces;
    std::vector<CastleButtonEntry> buttons;
    bool taxArrows = false; // the Tax tab's four newui_Bt_scroll_up / dn arrows
    Rml::String exitTooltip;
};
} // namespace mu::ui::window
