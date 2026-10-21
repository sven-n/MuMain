#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemGlowColorsUI.h"

#include "ItemEdits.h"
#include "ItemSelection.h"
#include "ItemToolWindow.h"
#include "ItemUsers.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "I18N/All.h"
#include "imgui.h"

#include <cstdio>
#include <string>

using namespace MuEditor::Items;
using Data::Effects::GlowColor;
using Data::Effects::GlowColorValue;

namespace
{
constexpr char ConfirmPopupId[] = "###GlowColorChange";
// Pixel widths before the editor UI scale.
constexpr float ColorListWidth = 220.0f;
constexpr float ValueWidth = 260.0f;
constexpr float UserListLines = 8.0f;

// Items without colors of their own glow with these (ItemGlow), so they stay.
bool IsDefaultColor(const std::string& name)
{
    using Data::Items::ItemGlow;
    return name == ItemGlow::DefaultColor || name == ItemGlow::DefaultShineColor ||
           name == ItemGlow::DefaultAncientColor;
}

void Swatch(const GlowColorValue& value, const char* id)
{
    const std::array<float, 3> rgb = ToColorField(value);
    ImGui::ColorButton(id, ImVec4(rgb[0], rgb[1], rgb[2], 1.0f), ImGuiColorEditFlags_NoTooltip);
}

// The items as a list; a click selects one.
void RenderUsers(const char* id, const std::vector<int>& users)
{
    if (!ImGui::BeginListBox(id, ImVec2(-FLT_MIN, UserListLines * ImGui::GetTextLineHeightWithSpacing())))
    {
        return;
    }
    for (const int user : users)
    {
        ImGui::PushID(user);
        if (ImGui::Selectable(g_ItemDatabase.GetLogName(user).c_str(), user == g_ItemSelection.GetItemType()))
        {
            g_ItemSelection.Select(user);
        }
        ImGui::PopID();
    }
    ImGui::EndListBox();
}

// The value as the color field shows and stores it.
GlowColorValue AsShown(const GlowColorValue& value)
{
    return ToGlowColorValue(ToColorField(value));
}

// A grey line that wraps at the window's edge.
void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}
} // namespace

CMuItemGlowColorsUI& CMuItemGlowColorsUI::GetInstance()
{
    static CMuItemGlowColorsUI instance;
    return instance;
}

void CMuItemGlowColorsUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::GlowColors, MuEditor::WindowIds::ItemGlowColors, open))
    {
        if (ImGui::Button(I18N::Editor::NewGlowColor))
        {
            m_dialog.Open(m_picked);
        }
        if (const std::optional<GlowColor> created = m_dialog.Render())
        {
            g_ItemEdits.SetGlowColor(I18N::Editor::NewGlowColor, *created);
            m_picked = created->name;
        }
        RenderNote(I18N::Editor::TheItemModelsNameTheseColorsTheModelToolPicksAnItemSColors);
        RenderUnused();
        ImGui::Separator();

        const ImVec2 listSize(ColorListWidth * g_MuEditorCore.GetUIScale(), 0.0f);
        if (ImGui::BeginChild("##Colors", listSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
        {
            RenderList();
        }
        ImGui::EndChild();
        ImGui::SameLine();
        if (ImGui::BeginChild("##Picked"))
        {
            RenderPicked();
            RenderConfirm();
        }
        ImGui::EndChild();
    }
    ToolWindow::End();
}

void CMuItemGlowColorsUI::RenderList()
{
    for (const GlowColor& color : g_GlowColors.GetColors())
    {
        ImGui::PushID(color.name.c_str());
        Swatch(color.value, "##Swatch");
        ImGui::SameLine();
        const std::string label = color.name + " (" + std::to_string(m_counts.GlowColor(color.name)) + ")";
        if (ImGui::Selectable(label.c_str(), color.name == m_picked, MuEditor::KeyboardSelection::RowFlags))
        {
            m_picked = color.name;
        }
        ImGui::PopID();
    }
}

void CMuItemGlowColorsUI::RenderPicked()
{
    const GlowColorValue* value = g_GlowColors.Find(m_picked);
    if (value == nullptr)
    {
        ImGui::TextDisabled("%s", I18N::Editor::PickAColorInTheList);
        return;
    }
    UpdateEdit(*value);
    ImGui::TextUnformatted(m_picked.c_str());
    ImGui::SetNextItemWidth(ValueWidth * g_MuEditorCore.GetUIScale());
    ImGui::ColorEdit3("##Value", m_edit.data(), ImGuiColorEditFlags_Float);

    const std::vector<int> users = FindGlowColorUsers(m_picked);
    // As the field shows it: a value with more places than the field keeps
    // (written by hand) is not a change.
    const bool changed = ToGlowColorValue(m_edit) != AsShown(*value);
    ImGui::BeginDisabled(!changed);
    if (ImGui::Button(I18N::Editor::ApplyToTheGlowColor))
    {
        if (users.empty())
        {
            ApplyEdit();
        }
        else
        {
            // D26: a change to a color reaches all its items; they are listed
            // before it is applied.
            m_confirmUsers = users;
            ImGui::OpenPopup(ConfirmPopupId);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Revert))
    {
        m_edit = ToColorField(*value);
        m_loaded = AsShown(*value);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    // D26: only a color no item uses goes.
    ImGui::BeginDisabled(!users.empty() || IsDefaultColor(m_picked));
    if (ImGui::Button(I18N::Editor::Remove))
    {
        const std::string names[] = {m_picked};
        g_ItemEdits.RemoveGlowColors(I18N::Editor::RemoveAGlowColor, names);
    }
    ImGui::EndDisabled();
    if (IsDefaultColor(m_picked))
    {
        RenderNote(I18N::Editor::ADefaultColorItemsWithoutColorsOfTheirOwnGlowWithIt);
    }
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::UsedBy0Items, {std::to_string(users.size())}).c_str());
    RenderUsers("##Users", users);
}

void CMuItemGlowColorsUI::RenderConfirm()
{
    char title[128];
    std::snprintf(title, sizeof(title), "%s%s", I18N::Editor::ChangeAGlowColor, ConfirmPopupId);
    if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ImGui::TextUnformatted(
        I18N::Format(I18N::Editor::TheChangeReachesThese0Items, {std::to_string(m_confirmUsers.size())}).c_str());
    RenderUsers("##ConfirmUsers", m_confirmUsers);
    if (ImGui::Button(I18N::Editor::ApplyToTheGlowColor))
    {
        ApplyEdit();
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

// D26: a color is removed only while no item uses it; the defaults stay.
void CMuItemGlowColorsUI::RenderUnused()
{
    std::vector<std::string> unused;
    for (const GlowColor& color : g_GlowColors.GetColors())
    {
        if (m_counts.GlowColor(color.name) == 0 && !IsDefaultColor(color.name))
        {
            unused.push_back(color.name);
        }
    }
    if (unused.empty())
    {
        return;
    }
    std::string names;
    for (const std::string& name : unused)
    {
        names += (names.empty() ? "" : ", ") + name;
    }
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::NoItemUsesTheseGlowColors0, {names}).c_str());
    ImGui::PopTextWrapPos();
    if (ImGui::SmallButton(I18N::Editor::RemoveThem))
    {
        g_ItemEdits.RemoveGlowColors(I18N::Editor::RemoveUnusedGlowColors, unused);
    }
}

void CMuItemGlowColorsUI::ApplyEdit()
{
    m_loaded = ToGlowColorValue(m_edit);
    g_ItemEdits.SetGlowColor(I18N::Editor::ChangeAGlowColor, {m_picked, m_loaded});
}

void CMuItemGlowColorsUI::UpdateEdit(const GlowColorValue& value)
{
    const bool unchanged = ToGlowColorValue(m_edit) == m_loaded;
    if (m_editOf != m_picked || (m_editVersion != g_GlowColors.GetVersion() && unchanged))
    {
        m_edit = ToColorField(value);
        m_loaded = AsShown(value);
        m_editOf = m_picked;
    }
    m_editVersion = g_GlowColors.GetVersion();
}

#endif // _EDITOR
