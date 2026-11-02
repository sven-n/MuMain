#include "stdafx.h"

#ifdef _EDITOR

#include "GroundLabelStyleDialog.h"

#include "GroundLabelWidgets.h"

#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "I18N/All.h"
#include "imgui.h"

#include <cstdio>

namespace MuEditor::Colors
{
namespace
{
constexpr char PopupId[] = "###NewGroundLabelStyle";
// Pixel width before the editor UI scale.
constexpr float NameWidth = 260.0f;
} // namespace

bool IsFreeGroundLabelStyleName(const std::string& name)
{
    return Data::Items::Json::IsName(name) && g_ItemDatabase.GetGroundLabels().FindStyle(name) == nullptr;
}

void GroundLabelStyleDialog::Open(std::string_view from)
{
    const Data::Items::GroundLabelStyle* style = g_ItemDatabase.GetGroundLabels().FindStyle(from);
    m_style = style != nullptr ? *style : Data::Items::GroundLabelStyle{};
    m_style.name.clear();
    ImGui::OpenPopup(PopupId);
}

std::optional<Data::Items::GroundLabelStyle> GroundLabelStyleDialog::Render()
{
    char title[128];
    std::snprintf(title, sizeof(title), "%s%s", I18N::Editor::NewLabelStyle, PopupId);
    if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return std::nullopt;
    }
    std::optional<Data::Items::GroundLabelStyle> created;
    ImGui::SetNextItemWidth(NameWidth);
    if (ImGui::IsWindowAppearing())
    {
        ImGui::SetKeyboardFocusHere();
    }
    MuEditor::Items::InputString(I18N::Editor::Name, m_style.name);
    const bool canCreate = IsFreeGroundLabelStyleName(m_style.name);
    if (!canCreate)
    {
        ImGui::TextDisabled("%s", I18N::Editor::ANameOfLettersAndDigitsThatNoOtherLabelStyleHas);
    }
    GroundLabelWidgets::RenderStyleFields(m_style);
    GroundLabelWidgets::RenderLabel(m_style.name.empty() ? I18N::Editor::NewLabelStyle : m_style.name.c_str(),
                                    m_style.color, m_style.background, m_style.bold);
    ImGui::Separator();
    ImGui::BeginDisabled(!canCreate);
    if (ImGui::Button(I18N::Editor::Add))
    {
        created = m_style;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
    return created;
}
} // namespace MuEditor::Colors

#endif // _EDITOR
