#include "stdafx.h"

#ifdef _EDITOR

#include "WrongSideButtons.h"

#include "I18N/All.h"
#include "imgui.h"

namespace MuEditor::Items::WrongSideButtons
{
namespace
{
// A column as wide as its head (the table adds the cell padding itself).
void SetupColumn(const char* head)
{
    ImGui::TableSetupColumn(head, ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize(head).x);
}

// The alpha of a button that shows an item's mark faint.
constexpr float ItemMarkAlpha = 0.5f;

// Whether the button was clicked.
bool RenderButton(const char* id, bool selected, Shown shown, const char* tooltip)
{
    ImGui::TableNextColumn();
    const bool faint = selected && shown != Shown::Own;
    if (faint)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * ItemMarkAlpha);
        tooltip = shown == Shown::OfItem ? I18N::Editor::WhatAMarkOfTheItemMeans
                                         : I18N::Editor::WhatAMarkOfTheItemWithMarkedFieldsMeans;
    }
    const bool clicked = ImGui::RadioButton(id, selected);
    if (faint)
    {
        ImGui::PopStyleVar();
    }
    ImGui::SetItemTooltip("%s", tooltip);
    return clicked;
}

// A click on the selected side clears it.
WrongSide Toggle(WrongSide current, WrongSide clicked)
{
    return current == clicked ? WrongSide::None : clicked;
}
} // namespace

void SetupColumns()
{
    SetupColumn(I18N::Editor::WrongInClient);
    SetupColumn(I18N::Editor::WrongInServer);
}

std::optional<WrongSide> Render(WrongSide side, Shown shown)
{
    std::optional<WrongSide> picked;
    if (RenderButton("##WrongInClient", side == WrongSide::Client, shown, I18N::Editor::WhatWrongInClientMeans))
    {
        picked = Toggle(side, WrongSide::Client);
    }
    if (RenderButton("##WrongInServer", side == WrongSide::Server, shown, I18N::Editor::WhatWrongInServerMeans))
    {
        picked = Toggle(side, WrongSide::Server);
    }
    if (picked == WrongSide::None)
    {
        switch (shown)
        {
        case Shown::OfItem:
            // The item's mark is not the field's to clear.
            return std::nullopt;
        case Shown::OfItemWithMarkedFields:
            // Every field takes the item's mark.
            return side;
        default:
            break;
        }
    }
    return picked;
}
} // namespace MuEditor::Items::WrongSideButtons

#endif // _EDITOR
