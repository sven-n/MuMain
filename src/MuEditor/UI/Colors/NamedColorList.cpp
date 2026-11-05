#include "stdafx.h"

#ifdef _EDITOR

#include "NamedColorList.h"

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
constexpr float UserListLines = 8.0f;

// The users as a list; a click on one runs its select.
void RenderUsers(const char* id, const std::vector<NamedColorUser>& users)
{
    if (!ImGui::BeginListBox(id, ImVec2(-FLT_MIN, UserListLines * ImGui::GetTextLineHeightWithSpacing())))
    {
        return;
    }
    for (std::size_t index = 0; index < users.size(); ++index)
    {
        const NamedColorUser& user = users[index];
        ImGui::PushID(static_cast<int>(index));
        if (ImGui::Selectable(user.label.c_str(), user.selected) && user.select)
        {
            user.select();
        }
        ImGui::PopID();
    }
    ImGui::EndListBox();
}
} // namespace

std::vector<std::string> GetNames(const NamedColorSource& source)
{
    std::vector<std::string> names;
    names.reserve(source.GetCount());
    for (std::size_t index = 0; index < source.GetCount(); ++index)
    {
        names.push_back(source.GetName(index));
    }
    return names;
}

std::vector<NamedColorUser> MakeItemUsers(std::span<const int> itemTypes)
{
    std::vector<NamedColorUser> users;
    users.reserve(itemTypes.size());
    for (const int itemType : itemTypes)
    {
        users.push_back({g_ItemDatabase.GetLogName(itemType), [itemType] { g_ItemSelection.Select(itemType); },
                         itemType == g_ItemSelection.GetItemType()});
    }
    return users;
}

void RenderColorNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}

std::string NamedColorList::RenderEntry(NamedColorSource& source, const std::string& name)
{
    const NamedColorTexts texts = source.GetTexts();
    ImGui::TextUnformatted(name.c_str());
    source.RenderEdit(name);

    std::string after = name;
    const std::vector<NamedColorUser> users = source.FindUsers(name);
    ImGui::BeginDisabled(!source.IsEdited(name));
    if (ImGui::Button(texts.apply))
    {
        if (users.empty())
        {
            after = source.GetNameAfterEdit(name);
            source.ApplyEdit(name);
        }
        else
        {
            // D26: a change to an entry reaches all its users; they are listed before it is applied.
            m_confirmOf = name;
            m_confirmUsers = users;
            ImGui::OpenPopup(ConfirmPopupId);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Revert))
    {
        source.RevertEdit(name);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    // D26: only an entry nothing uses goes.
    const bool kept = source.IsKept(name);
    ImGui::BeginDisabled(!users.empty() || kept);
    if (ImGui::Button(I18N::Editor::Remove))
    {
        const std::string names[] = {name};
        source.Remove(texts.removeOne, names);
        after.clear();
    }
    ImGui::EndDisabled();
    if (kept)
    {
        RenderColorNote(texts.kept);
    }
    ImGui::TextUnformatted(I18N::Format(texts.usedBy, {std::to_string(users.size())}).c_str());
    RenderUsers("##Users", users);
    if (std::string confirmed = RenderConfirm(source, texts); !confirmed.empty())
    {
        after = std::move(confirmed);
    }
    return after;
}

std::string NamedColorList::RenderConfirm(NamedColorSource& source, const NamedColorTexts& texts)
{
    char title[128];
    std::snprintf(title, sizeof(title), "%s%s", texts.change, ConfirmPopupId);
    if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return {};
    }
    std::string applied;
    ImGui::TextUnformatted(I18N::Format(texts.changeReaches, {std::to_string(m_confirmUsers.size())}).c_str());
    RenderUsers("##ConfirmUsers", m_confirmUsers);
    if (ImGui::Button(texts.apply))
    {
        if (source.Has(m_confirmOf))
        {
            applied = source.GetNameAfterEdit(m_confirmOf);
            source.ApplyEdit(m_confirmOf);
        }
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
    return applied;
}

// D26: an entry is removed only while nothing uses it; the kept ones stay.
void NamedColorList::RenderUnused(NamedColorSource& source)
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
    const NamedColorTexts texts = source.GetTexts();
    RenderColorNote(I18N::Format(texts.unused, {names}).c_str());
    if (ImGui::SmallButton(I18N::Editor::RemoveThem))
    {
        source.Remove(texts.removeUnused, unused);
    }
}
} // namespace MuEditor::Colors

#endif // _EDITOR
