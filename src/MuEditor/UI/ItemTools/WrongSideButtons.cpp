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

// Whether the button was clicked.
bool RenderButton(const char* id, bool selected, const char* tooltip)
{
    ImGui::TableNextColumn();
    const bool clicked = ImGui::RadioButton(id, selected);
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", tooltip);
    }
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

std::optional<WrongSide> Render(WrongSide side)
{
    std::optional<WrongSide> picked;
    if (RenderButton("##WrongInClient", side == WrongSide::Client, I18N::Editor::WhatWrongInClientMeans))
    {
        picked = Toggle(side, WrongSide::Client);
    }
    if (RenderButton("##WrongInServer", side == WrongSide::Server, I18N::Editor::WhatWrongInServerMeans))
    {
        picked = Toggle(side, WrongSide::Server);
    }
    return picked;
}
} // namespace MuEditor::Items::WrongSideButtons

#endif // _EDITOR
