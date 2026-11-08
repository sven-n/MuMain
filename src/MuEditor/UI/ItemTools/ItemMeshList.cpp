#include "stdafx.h"

#ifdef _EDITOR

#include "ItemMeshList.h"

#include "ItemEdits.h"
#include "ItemMeshFocus.h"
#include "ItemTextInput.h"
#include "ItemToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/SourceText.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "I18N/All.h"
#include "Render/Models/ZzzBMD.h"
#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <map>
#include <string>
#include <utility>

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemModelDefinition;

constexpr int ColumnCount = 5;
// Pixel width of the name column before the editor UI scale.
constexpr float NameWidth = 110.0f;
constexpr float MaxVisibleRows = 8.0f;
// The header row and a little room below the last row.
constexpr float ExtraRows = 1.5f;
constexpr const char* NumberColumn = "#";
constexpr const char* Separator = ", ";
constexpr const char* LabelSeparator = ": ";
// No data version: the rows are read again.
constexpr long long NotRead = -1;

// A grey line that wraps at the window's edge.
void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}

void Append(std::string& text, const char* part)
{
    if (!text.empty())
    {
        text += Separator;
    }
    text += part;
}

// The marks in the editor's language now: it can change while the rows stay.
void BuildMarksText(const ItemMeshRow& row, std::string& marks)
{
    marks.clear();
    const std::pair<bool, const char*> all[] = {
        {row.hidden, I18N::Editor::MeshHidden},   {row.glows, I18N::Editor::MeshGlows},
        {row.scrolls, I18N::Editor::MeshScrolls}, {row.noChrome, I18N::Editor::MeshNoChrome},
        {row.holes, I18N::Editor::MeshHoles},
    };
    for (const auto& [marked, label] : all)
    {
        if (marked)
        {
            Append(marks, label);
        }
    }
}

void RenderTooltip(const ItemMeshRow& row)
{
    if (!ImGui::BeginTooltip())
    {
        return;
    }
    if (row.textureFile.empty())
    {
        ImGui::TextUnformatted(I18N::Editor::MeshTextureNotLoaded);
    }
    else
    {
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::MeshTextureFile0, {row.textureFile}).c_str());
        const std::string width = std::to_string(row.textureWidth);
        const std::string height = std::to_string(row.textureHeight);
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::MeshTextureSize01, {width, height}).c_str());
    }
    const std::string vertices = std::to_string(row.vertices);
    const std::string triangles = std::to_string(row.triangles);
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::MeshCounts01, {vertices, triangles}).c_str());
    if (row.hidden)
    {
        ImGui::TextDisabled("%s", I18N::Editor::MeshNeverDrawn);
    }
    ImGui::EndTooltip();
}
} // namespace

void CItemMeshList::Render(int itemType, ItemModelDefinition& entry, ItemEditDraft<ItemModelDefinition>& draft,
                           bool inPreview)
{
    Refresh(itemType, Models[Data::Items::ToModelSlot(itemType)]);
    // Also while the list is folded: a kept mesh the model lost would leave every mesh see-through.
    g_ItemMeshFocus.ForgetMeshesFrom(static_cast<int>(m_rows.size()));
    if (!ImGui::CollapsingHeader(I18N::Editor::Meshes, ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    if (m_rows.empty())
    {
        RenderNote(I18N::Editor::NoMeshesLoaded);
        return;
    }
    RenderNote(inPreview ? I18N::Editor::MeshListNote : I18N::Editor::MeshesNotInPreviewNote);
    RenderTable(entry, draft);
    if (!entry.renderStyle.empty() || !entry.itemEffect.empty())
    {
        RenderNote(I18N::Editor::MeshCodeLooksNote);
    }
}

void CItemMeshList::Refresh(int itemType, const BMD& model)
{
    const long long dataVersion = ToolWindow::GetDataVersion();
    if (itemType == m_itemType && model.Meshs == m_meshes && model.IndexTexture == m_textures &&
        model.NumMeshs == m_meshCount && dataVersion == m_dataVersion)
    {
        return;
    }
    m_itemType = itemType;
    m_meshes = model.Meshs;
    m_textures = model.IndexTexture;
    m_meshCount = model.NumMeshs;
    m_dataVersion = dataVersion;
    m_rows = DescribeMeshes(model);
    // An edit's model is opened again between frames, often into the same memory, so nothing above changes: the
    // rows read before that are read again in the next frame.
    if (g_ItemDataHandler.HasModelsToReopen())
    {
        m_dataVersion = NotRead;
    }
}

void CItemMeshList::RenderTable(ItemModelDefinition& entry, ItemEditDraft<ItemModelDefinition>& draft)
{
    const float visibleRows = std::min(static_cast<float>(m_rows.size()), MaxVisibleRows) + ExtraRows;
    const ImVec2 size(0.0f, visibleRows * ImGui::GetTextLineHeightWithSpacing());
    const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable;
    if (!ImGui::BeginTable("##Meshes", ColumnCount, flags, size))
    {
        return;
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn(NumberColumn);
    ImGui::TableSetupColumn(I18N::Editor::Name, ImGuiTableColumnFlags_WidthFixed,
                            NameWidth * g_MuEditorCore.GetUIScale());
    ImGui::TableSetupColumn(I18N::Editor::MeshTexture);
    ImGui::TableSetupColumn(I18N::Editor::MeshMarks);
    ImGui::TableSetupColumn(I18N::Editor::MeshUsedBy, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();
    const bool scrollToKept = g_ItemMeshFocus.TakePicked();
    for (std::size_t index = 0; index < m_rows.size(); ++index)
    {
        RenderRow(index, FindMeshUses(entry, m_rows[index].mesh), scrollToKept, entry, draft);
    }
    ImGui::EndTable();
}

void CItemMeshList::RenderRow(std::size_t index, MeshUses uses, bool scrollToKept, ItemModelDefinition& entry,
                              ItemEditDraft<ItemModelDefinition>& draft)
{
    const ItemMeshRow& row = m_rows[index];
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushID(row.mesh);
    char number[16];
    std::snprintf(number, sizeof(number), "%d", row.mesh);
    const bool kept = g_ItemMeshFocus.Kept() == row.mesh;
    // The name field in the row takes its own clicks.
    if (ImGui::Selectable(number, kept, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
    {
        g_ItemMeshFocus.ToggleKept(row.mesh);
    }
    if (ImGui::IsItemHovered())
    {
        g_ItemMeshFocus.Point(row.mesh);
        RenderTooltip(row);
    }
    if (kept && scrollToKept)
    {
        ImGui::SetScrollHereY();
    }
    ImGui::TableNextColumn();
    RenderName(row.mesh, entry, draft);
    ImGui::PopID();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(row.texture.c_str());
    ImGui::TableNextColumn();
    BuildMarksText(row, m_marksText);
    ImGui::TextUnformatted(m_marksText.c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(UsesText(uses).c_str());
}

void CItemMeshList::RenderName(int mesh, ItemModelDefinition& entry, ItemEditDraft<ItemModelDefinition>& draft)
{
    const auto named = entry.meshNames.find(mesh);
    if (m_nameMesh != mesh)
    {
        m_nameText = named != entry.meshNames.end() ? named->second : std::string();
    }
    ImGui::SetNextItemWidth(-FLT_MIN);
    InputStringWithHint("##Name", I18N::Editor::NameThisPart, m_nameText);
    if (ImGui::IsItemActive())
    {
        m_nameMesh = mesh;
    }
    if (!ImGui::IsItemDeactivated())
    {
        return;
    }
    m_nameMesh = -1;
    const std::string name = SourceText::Trimmed(m_nameText);
    const std::string current = named != entry.meshNames.end() ? named->second : std::string();
    // A name that can't be one is not stored; the field shows the stored one again.
    if (name == current || (!name.empty() && !Data::Colors::IsColorFolderName(name)))
    {
        return;
    }
    std::map<int, std::string> names = entry.meshNames;
    if (name.empty())
    {
        names.erase(mesh);
    }
    else
    {
        names[mesh] = name;
    }
    if (const Data::Items::SharedItemModel* shared = g_ItemModelDatabase.FindSharedModel(entry.model))
    {
        // The file's names are the shared model's, as the file is.
        Data::Items::SharedItemModel copy = *shared;
        copy.meshNames = std::move(names);
        g_ItemEdits.SetSharedModel(I18N::Editor::NameAPart, copy);
        return;
    }
    entry.meshNames = std::move(names);
    draft.AfterChange(true, I18N::Editor::NameAPart);
}

// The labels of the mesh fields, as in the Model file and Glow sections; built into one reused string.
const std::string& CItemMeshList::UsesText(MeshUses uses)
{
    m_usesText.clear();
    const auto add = [&](MeshUses use, const char* label, bool allBut)
    {
        if ((uses & use) == 0)
        {
            return;
        }
        Append(m_usesText, label);
        if (allBut)
        {
            m_usesText += LabelSeparator;
            m_usesText += I18N::Editor::MeshAllButThisOne;
        }
    };
    add(MeshUse::NoneBlend, I18N::Editor::NoneBlendMeshes, false);
    add(MeshUse::Glow, I18N::Editor::GlowMeshes, false);
    add(MeshUse::GlowAllBut, I18N::Editor::GlowMeshes, true);
    add(MeshUse::Shine, I18N::Editor::ShineMeshes, false);
    add(MeshUse::ShineAllBut, I18N::Editor::ShineMeshes, true);
    add(MeshUse::Excellent, I18N::Editor::ExcellentMesh, false);
    add(MeshUse::ExcellentWithoutSkin, I18N::Editor::ExcellentMeshWithoutSkin, false);
    return m_usesText;
}
} // namespace MuEditor::Items

#endif // _EDITOR
