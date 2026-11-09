#include "stdafx.h"

#ifdef _EDITOR

#include "WrongSideButtons.h"

#include "I18N/All.h"
#include "imgui.h"

namespace MuEditor::Items::WrongSideButtons
{
namespace
{
// A column as wide as its head.
void SetupColumn(const char* head)
{
    const float width = ImGui::CalcTextSize(head).x + ImGui::GetStyle().CellPadding.x * 2.0f;
    ImGui::TableSetupColumn(head, ImGuiTableColumnFlags_WidthFixed, width);
}

// The alpha of a field's button that shows its item's mark.
constexpr float ItemMarkAlpha = 0.5f;

// Whether the button was clicked.
bool RenderButton(const char* id, bool selected, bool ofItem, const char* tooltip)
{
    ImGui::TableNextColumn();
    const bool faint = selected && ofItem;
    if (faint)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * ItemMarkAlpha);
        tooltip = I18N::Editor::WhatAMarkOfTheItemMeans;
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

std::optional<WrongSide> Render(WrongSide side, bool ofItem)
{
    std::optional<WrongSide> picked;
    if (RenderButton("##WrongInClient", side == WrongSide::Client, ofItem, I18N::Editor::WhatWrongInClientMeans))
    {
        picked = Toggle(side, WrongSide::Client);
    }
    if (RenderButton("##WrongInServer", side == WrongSide::Server, ofItem, I18N::Editor::WhatWrongInServerMeans))
    {
        picked = Toggle(side, WrongSide::Server);
    }
    // The item's mark is not the field's to clear.
    if (ofItem && picked == WrongSide::None)
    {
        return std::nullopt;
    }
    return picked;
}
} // namespace MuEditor::Items::WrongSideButtons

#endif // _EDITOR
