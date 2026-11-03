#include "stdafx.h"

#ifdef _EDITOR

#include "ItemSharedModelForm.h"

#include "ItemEdits.h"
#include "ItemListFields.h"
#include "ItemNumberList.h"
#include "ItemSelection.h"
#include "ItemTextInput.h"
#include "ItemUsers.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>

namespace MuEditor::Items
{
namespace
{
constexpr char ConfirmPopupId[] = "###SharedModelChange";
constexpr float FieldWidth = 260.0f;
constexpr float UserListLines = 6.0f;

// The names the model files take: letters and digits, a letter first.
bool IsSharedModelName(const std::string& name)
{
    const auto isLetter = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); };
    if (name.empty() || !isLetter(name.front()))
    {
        return false;
    }
    return std::ranges::all_of(name, [&](char c) { return isLetter(c) || (c >= '0' && c <= '9'); });
}

std::string CountedName(const std::string& name, int count)
{
    return name + " (" + std::to_string(count) + ")";
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
} // namespace

void CItemSharedModelForm::Render(const Data::Items::ItemModelDefinition& model, ItemLookUseCounts& counts)
{
    if (model.model.empty())
    {
        RenderPick(model, counts);
    }
    else
    {
        RenderShared(model);
    }
    RenderUnused(counts);
}

// D26: a shared model is removed only while no item uses it.
void CItemSharedModelForm::RenderUnused(ItemLookUseCounts& counts)
{
    std::vector<std::string> unused;
    for (const Data::Items::SharedItemModel& shared : g_ItemModelDatabase.GetSharedModels())
    {
        if (counts.SharedModel(shared.name) == 0)
        {
            unused.push_back(shared.name);
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
    ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::NoItemUsesTheseSharedModels0, {names}).c_str());
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::RemoveThem))
    {
        g_ItemEdits.BeginGroup(I18N::Editor::RemoveUnusedSharedModels);
        for (const std::string& name : unused)
        {
            g_ItemEdits.RemoveSharedModel(I18N::Editor::RemoveUnusedSharedModels, name);
        }
        g_ItemEdits.EndGroup();
    }
}

void CItemSharedModelForm::RenderPick(const Data::Items::ItemModelDefinition& model, ItemLookUseCounts& counts)
{
    ImGui::SetNextItemWidth(FieldWidth);
    if (!ImGui::BeginCombo(I18N::Editor::UseASharedModel, I18N::Editor::None))
    {
        return;
    }
    for (const Data::Items::SharedItemModel& shared : g_ItemModelDatabase.GetSharedModels())
    {
        if (ImGui::Selectable(CountedName(shared.name, counts.SharedModel(shared.name)).c_str(), false))
        {
            Data::Items::ItemModelDefinition used = model;
            used.model = shared.name;
            g_ItemEdits.SetItemModel(I18N::Editor::SharedModel, used);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", shared.file.c_str());
        }
    }
    ImGui::EndCombo();
}

void CItemSharedModelForm::RenderShared(const Data::Items::ItemModelDefinition& model)
{
    const Data::Items::SharedItemModel* shared = g_ItemModelDatabase.FindSharedModel(model.model);
    if (shared == nullptr)
    {
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::TheSharedModel0DoesNotExist, {model.model}).c_str());
        return;
    }
    // The copy follows the shared model (an undo, a redo) until the user
    // changes it.
    if (m_editOf != model.model || (m_editVersion != g_ItemModelDatabase.GetVersion() && IsEditUnchanged()))
    {
        LoadEdit(*shared);
    }
    m_editVersion = g_ItemModelDatabase.GetVersion();

    const std::vector<int> users = FindSharedModelUsers(model.model);
    ImGui::TextUnformatted(
        I18N::Format(I18N::Editor::SharedModel0AlsoUsedBy1Items, {model.model, std::to_string(users.size() - 1)})
            .c_str());
    RenderUsers("##SharedModelUsers", users);
    RenderEdit();

    const bool changed = !(m_edit == *shared);
    const bool nameFree = m_edit.name == m_editOf || g_ItemModelDatabase.FindSharedModel(m_edit.name) == nullptr;
    const bool nameValid = IsSharedModelName(m_edit.name) && nameFree;
    if (!nameValid)
    {
        ImGui::TextDisabled("%s", I18N::Editor::ANameOfLettersAndDigitsThatNoOtherSharedModelHas);
    }
    ImGui::BeginDisabled(!changed || !nameValid);
    if (ImGui::Button(I18N::Editor::ApplyToTheSharedModel))
    {
        m_confirmUsers = users;
        m_openConfirm = true;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    // Also with a name that cannot be applied: that is when it is needed.
    ImGui::BeginDisabled(!changed);
    if (ImGui::Button(I18N::Editor::Revert))
    {
        LoadEdit(*shared);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::MakeACopyForThisItem))
    {
        // A variant for one item: it keeps the file and folders as its own.
        Data::Items::ItemModelDefinition own = model;
        own.model.clear();
        g_ItemEdits.SetItemModel(I18N::Editor::MakeACopyForThisItem, own);
    }
}

void CItemSharedModelForm::RenderEdit()
{
    // Its fields are named like the model tool's headers (Model file).
    ImGui::PushID("SharedModel");
    ImGui::SetNextItemWidth(FieldWidth);
    InputString(I18N::Editor::SharedModelName, m_edit.name);
    ImGui::SetNextItemWidth(FieldWidth);
    InputString(I18N::Editor::ModelFile, m_edit.file);
    ImGui::TextUnformatted(I18N::Editor::TextureFolders);
    ListFields::EditFolders("##SharedFolders", m_edit.textureFolders);
    ImGui::SetNextItemWidth(FieldWidth);
    if (InputString(I18N::Editor::NoneBlendMeshes, m_meshesText))
    {
        if (const std::optional<std::vector<int>> meshes = NumberList::Parse(m_meshesText))
        {
            m_edit.noneBlendMeshes = *meshes;
        }
    }
    ImGui::PopID();
}

// D26: a change to a shared model reaches all its items; they are listed
// before it is applied.
void CItemSharedModelForm::RenderConfirm()
{
    if (m_openConfirm)
    {
        ImGui::OpenPopup(ConfirmPopupId);
        m_openConfirm = false;
    }
    char title[128];
    std::snprintf(title, sizeof(title), "%s%s", I18N::Editor::ChangeASharedModel, ConfirmPopupId);
    if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    ImGui::TextUnformatted(
        I18N::Format(I18N::Editor::TheChangeReachesThese0Items, {std::to_string(m_confirmUsers.size())}).c_str());
    RenderUsers("##ConfirmUsers", m_confirmUsers);
    if (ImGui::Button(I18N::Editor::ApplyToTheSharedModel))
    {
        g_ItemEdits.SetSharedModel(I18N::Editor::ChangeASharedModel, m_edit, m_editOf);
        m_editOf = m_edit.name;
        m_loaded = m_edit;
        // As LoadEdit writes it: the applied copy is unchanged and follows an
        // undo or redo.
        m_meshesText = NumberList::Format(m_edit.noneBlendMeshes);
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void CItemSharedModelForm::LoadEdit(const Data::Items::SharedItemModel& shared)
{
    m_edit = shared;
    m_loaded = shared;
    m_editOf = shared.name;
    m_meshesText = NumberList::Format(shared.noneBlendMeshes);
}

bool CItemSharedModelForm::IsEditUnchanged() const
{
    return m_edit == m_loaded && m_meshesText == NumberList::Format(m_loaded.noneBlendMeshes);
}
} // namespace MuEditor::Items

#endif // _EDITOR
