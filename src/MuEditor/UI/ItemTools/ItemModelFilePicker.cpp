#include "stdafx.h"

#ifdef _EDITOR

#include "ItemModelFilePicker.h"

#include "ItemEdits.h"
#include "ItemSelection.h"
#include "ItemTextInput.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>

namespace MuEditor::Items
{
namespace
{
constexpr float ListLines = 8.0f;
constexpr float SearchWidth = 260.0f;

const Data::Items::SharedItemModel* FindSharedModelWithFile(std::string_view file)
{
    for (const Data::Items::SharedItemModel& shared : g_ItemModelDatabase.GetSharedModels())
    {
        if (shared.file == file)
        {
            return &shared;
        }
    }
    return nullptr;
}
} // namespace

void CItemModelFilePicker::Render(int itemType, const Data::Items::ItemModelDefinition* model)
{
    ImGui::SetNextItemWidth(SearchWidth);
    InputStringWithHint("##ModelFileSearch", I18N::Editor::FindAModelFile, m_search);
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::Refresh))
    {
        m_files.Refresh();
        m_foundValid = false;
    }
    if (!m_foundValid || m_foundFor != m_search)
    {
        m_found = m_files.Find(m_search);
        m_foundFor = m_search;
        m_foundValid = true;
    }
    if (ImGui::BeginListBox("##ModelFiles", ImVec2(-FLT_MIN, ListLines * ImGui::GetTextLineHeightWithSpacing())))
    {
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(m_found.size()));
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                const std::string_view file = m_found[row];
                const bool current = model != nullptr && model->file == file;
                ImGui::PushID(row);
                if (ImGui::Selectable(std::string(file).c_str(), current) && !current)
                {
                    Pick(itemType, model, file);
                }
                ImGui::PopID();
            }
        }
        ImGui::EndListBox();
    }
    if (model != nullptr)
    {
        RenderOtherUsers(itemType, *model);
    }
}

void CItemModelFilePicker::Pick(int itemType, const Data::Items::ItemModelDefinition* model, std::string_view file)
{
    Data::Items::ItemModelDefinition picked;
    if (model != nullptr)
    {
        picked = *model;
    }
    picked.group = Data::Items::GetItemGroup(itemType);
    picked.number = Data::Items::GetItemNumber(itemType);
    picked.model.clear();
    // A model from another folder takes its textures from there too, unless
    // the folders were chosen.
    picked.textureFolders = ItemModelFiles::GetTextureFoldersForFile(picked.textureFolders, picked.file, file);
    picked.file = std::string(file);
    g_ItemEdits.SetItemModel(I18N::Editor::ModelFile, picked);
}

// Other items that open the same file: through a shared model the item can
// use too, or each by itself, which a shared model would make one opening.
void CItemModelFilePicker::RenderOtherUsers(int itemType, const Data::Items::ItemModelDefinition& model)
{
    if (!model.model.empty() || model.file.empty())
    {
        return;
    }
    if (const Data::Items::SharedItemModel* shared = FindSharedModelWithFile(model.file))
    {
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::TheSharedModel0OpensThisFile, {shared->name}).c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::UseIt))
        {
            Data::Items::ItemModelDefinition used = model;
            used.model = shared->name;
            g_ItemEdits.SetItemModel(I18N::Editor::SharedModel, used);
        }
        return;
    }
    int others = 0;
    for (const Data::Items::ItemModelDefinition& other : g_ItemModelDatabase.GetAllSlots())
    {
        others += other.Exists() && other.model.empty() && other.file == model.file &&
                          Data::Items::MakeItemType(other.group, other.number) != itemType
                      ? 1
                      : 0;
    }
    if (others > 0)
    {
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::OtherItemsNameThisFileToo0ASharedModelWouldOpenItOnce,
                                               {std::to_string(others)})
                                      .c_str());
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
