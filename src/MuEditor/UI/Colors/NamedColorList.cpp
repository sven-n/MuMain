#include "stdafx.h"

#ifdef _EDITOR

#include "NamedColorList.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/ItemTools/ItemSelection.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <cstdio>

namespace MuEditor::Colors
{
namespace
{
constexpr char ConfirmPopupId[] = "###NamedColorChange";
// Pixel width before the editor UI scale.
constexpr float ListWidth = 220.0f;
constexpr float UserListLines = 8.0f;

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

// A grey line that wraps at the window's edge.
void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}
} // namespace

void NamedColorList::Render(NamedColorSource& source)
{
    const NamedColorTexts texts = source.GetTexts();
    if (ImGui::Button(texts.newEntry))
    {
        source.OpenNew(m_picked);
    }
    if (std::optional<std::string> created = source.RenderNew())
    {
        m_picked = std::move(*created);
    }
    RenderNote(texts.note);
    RenderUnused(source, texts);
    ImGui::Separator();

    const ImVec2 listSize(ListWidth * g_MuEditorCore.GetUIScale(), 0.0f);
    if (ImGui::BeginChild("##Entries", listSize, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
    {
        RenderList(source);
    }
    ImGui::EndChild();
    ImGui::SameLine();
    if (ImGui::BeginChild("##Picked"))
    {
        RenderPicked(source, texts);
        RenderConfirm(source, texts);
    }
    ImGui::EndChild();
}

void NamedColorList::RenderList(NamedColorSource& source)
{
    for (std::size_t index = 0; index < source.GetCount(); ++index)
    {
        const std::string& name = source.GetName(index);
        ImGui::PushID(name.c_str());
        source.RenderSwatch(index);
        ImGui::SameLine();
        const std::string label = name + " (" + std::to_string(source.CountUsers(name)) + ")";
        if (ImGui::Selectable(label.c_str(), name == m_picked, MuEditor::KeyboardSelection::RowFlags))
        {
            m_picked = name;
        }
        ImGui::PopID();
    }
}

void NamedColorList::RenderPicked(NamedColorSource& source, const NamedColorTexts& texts)
{
    if (!source.Has(m_picked))
    {
        ImGui::TextDisabled("%s", texts.pickAnEntry);
        return;
    }
    ImGui::TextUnformatted(m_picked.c_str());
    source.RenderEdit(m_picked);

    const std::vector<int> users = source.FindUsers(m_picked);
    ImGui::BeginDisabled(!source.IsEdited(m_picked));
    if (ImGui::Button(texts.apply))
    {
        if (users.empty())
        {
            source.ApplyEdit(m_picked);
        }
        else
        {
            // D26: a change to an entry reaches all its items; they are listed before it is applied.
            m_confirmUsers = users;
            ImGui::OpenPopup(ConfirmPopupId);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Revert))
    {
        source.RevertEdit(m_picked);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    // D26: only an entry no item uses goes.
    const bool kept = source.IsKept(m_picked);
    ImGui::BeginDisabled(!users.empty() || kept);
    if (ImGui::Button(I18N::Editor::Remove))
    {
        const std::string names[] = {m_picked};
        source.Remove(texts.removeOne, names);
    }
    ImGui::EndDisabled();
    if (kept)
    {
        RenderNote(texts.kept);
    }
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::UsedBy0Items, {std::to_string(users.size())}).c_str());
    RenderUsers("##Users", users);
}

void NamedColorList::RenderConfirm(NamedColorSource& source, const NamedColorTexts& texts)
{
    char title[128];
    std::snprintf(title, sizeof(title), "%s%s", texts.change, ConfirmPopupId);
    if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ImGui::TextUnformatted(
        I18N::Format(I18N::Editor::TheChangeReachesThese0Items, {std::to_string(m_confirmUsers.size())}).c_str());
    RenderUsers("##ConfirmUsers", m_confirmUsers);
    if (ImGui::Button(texts.apply))
    {
        source.ApplyEdit(m_picked);
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

// D26: an entry is removed only while no item uses it; the kept ones stay.
void NamedColorList::RenderUnused(NamedColorSource& source, const NamedColorTexts& texts)
{
    std::vector<std::string> unused;
    for (std::size_t index = 0; index < source.GetCount(); ++index)
    {
        const std::string& name = source.GetName(index);
        if (source.CountUsers(name) == 0 && !source.IsKept(name))
        {
            unused.push_back(name);
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
    RenderNote(I18N::Format(texts.unused, {names}).c_str());
    if (ImGui::SmallButton(I18N::Editor::RemoveThem))
    {
        source.Remove(texts.removeUnused, unused);
    }
}
} // namespace MuEditor::Colors

#endif // _EDITOR
