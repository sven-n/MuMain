#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemModelUI.h"

#include "ItemEdits.h"
#include "ItemMeshRows.h"
#include "ItemTextInput.h"
#include "ItemLevelModels.h"
#include "ItemMeshFocus.h"
#include "ItemModelForms.h"
#include "ItemSelection.h"
#include "ItemToolWindow.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/StyleEditor/MuStyleEditorUI.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/PartModelSlots.h"
#include "Core/Text/Utf8.h"
#include "Data/DataHandler/LoadData.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "Render/Items/ItemDisplay.h"
#include "Render/Items/ItemRenderStyles.h"
#include "Render/Models/ZzzBMD.h"
#include "imgui.h"

#include <algorithm>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

using namespace MuEditor::Items;
using Data::Items::ItemModelDefinition;

namespace
{
constexpr ImVec4 RemoveColor{0.7f, 0.2f, 0.2f, 1.0f};
constexpr float PartFieldWidth = 320.0f;

// "Item, Item/partCharge1" and back.
std::string JoinFolders(const std::vector<std::string>& folders)
{
    std::string text;
    for (const std::string& folder : folders)
    {
        text += (text.empty() ? "" : ", ") + folder;
    }
    return text;
}

std::vector<std::string> SplitFolders(const std::string& text)
{
    std::vector<std::string> folders;
    std::stringstream stream(text);
    std::string folder;
    while (std::getline(stream, folder, ','))
    {
        const auto first = folder.find_first_not_of(' ');
        const auto last = folder.find_last_not_of(' ');
        if (first != std::string::npos)
        {
            folders.push_back(folder.substr(first, last - first + 1));
        }
    }
    return folders;
}

// The part models with the one of `name` changed by `change`, as one undo step.
template <typename TChange> void ChangePartModel(const char* label, const std::string& name, TChange&& change)
{
    const std::span<const Data::Items::PartModel> current = g_ItemModelDatabase.GetPartModels();
    std::vector<Data::Items::PartModel> parts(current.begin(), current.end());
    const auto part =
        std::ranges::find(parts, name, [](const Data::Items::PartModel& entry) { return entry.model.name; });
    if (part == parts.end())
    {
        return;
    }
    change(*part);
    g_ItemEdits.SetPartModels(label, std::move(parts));
}

std::optional<ItemModelDefinition> LoadModel(int itemType)
{
    const ItemModelDefinition* model = g_ItemModelDatabase.Find(itemType);
    return model != nullptr ? std::optional<ItemModelDefinition>(*model) : std::nullopt;
}

void StoreModel(const std::string& label, const ItemModelDefinition& model)
{
    g_ItemEdits.SetItemModel(label, model);
}
} // namespace

CMuItemModelUI& CMuItemModelUI::GetInstance()
{
    static CMuItemModelUI instance;
    return instance;
}

CMuItemModelUI::CMuItemModelUI() : m_draft(&LoadModel, &StoreModel) {}

void CMuItemModelUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::ItemModel, MuEditor::WindowIds::ItemModel, open))
    {
        const int itemType = g_ItemSelection.GetItemType();
        RenderPartModelList();
        if (!m_partModel.empty() && itemType != m_partModelItem)
        {
            // An item was picked: the window shows it again.
            m_partModel.clear();
        }
        if (!m_partModel.empty())
        {
            RenderPartModel(m_partModel);
            ToolWindow::End();
            return;
        }
        g_ItemMeshFocus.BeginFrame(itemType, ImGui::GetFrameCount());
        ItemModelDefinition* model = m_draft.Begin(itemType, ToolWindow::GetDataVersion());
        ShowInventoryDisplay(model, itemType);
        // Also a number with a model but no item can be edited here.
        ToolWindow::RenderSelectedItem(itemType);
        const bool valid = Data::Items::IsValidItemType(itemType);
        if (valid)
        {
            ImGui::PushID(itemType);
            RenderPreview(model, itemType);
            ImGui::PopID();
        }
        // The fields scroll below the preview, which stays where it is.
        if (ImGui::BeginChild("##Fields") && valid)
        {
            ImGui::PushID(itemType);
            RenderModel(model, itemType);
            ImGui::PopID();
        }
        ImGui::EndChild();
        // Outside the item's ID scope: the warning's list selects other items. Outside the fields too, which a window
        // as small as its preview leaves out.
        m_sharedForm.RenderConfirm();
        m_draft.End();
    }
    ToolWindow::End();
}

void CMuItemModelUI::RenderPartModelList()
{
    if (!ImGui::CollapsingHeader(I18N::Editor::PartModels))
    {
        return;
    }
    ImGui::PushID("PartModels");
    for (const Data::Items::PartModelSlot& slot : Data::Items::PartModelSlots)
    {
        const std::string name(slot.name);
        if (ImGui::Selectable(name.c_str(), m_partModel == name))
        {
            m_partModel = name;
            m_partModelItem = g_ItemSelection.GetItemType();
        }
    }
    ImGui::PopID();
    ImGui::Separator();
}

void CMuItemModelUI::RenderPartModel(const std::string& name)
{
    ImGui::PushID("PartModel");
    ImGui::TextUnformatted(name.c_str());
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::PartModelNote);
    ImGui::PopTextWrapPos();
    const Data::Items::PartModel* found = g_ItemModelDatabase.FindPartModel(name);
    const auto slot =
        std::ranges::find(Data::Items::PartModelSlots, std::string_view(name), &Data::Items::PartModelSlot::name);
    if (found == nullptr || slot == std::end(Data::Items::PartModelSlots))
    {
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::ThePartModel0HasNoEntry, {name}).c_str());
        ImGui::PopID();
        return;
    }
    // A copy: a change replaces the database's part models, which the fields after it still read this frame.
    const Data::Items::PartModel part = *found;
    if (ImGui::CollapsingHeader(I18N::Editor::Preview, ImGuiTreeNodeFlags_DefaultOpen))
    {
        m_preview.RenderModelSlot(slot->modelType);
    }
    if (ImGui::BeginChild("##PartFields"))
    {
        const float width = PartFieldWidth * ImGui::GetFontSize() / 13.0f;
        if (ImGui::CollapsingHeader(I18N::Editor::ModelFile, ImGuiTreeNodeFlags_DefaultOpen))
        {
            std::string typed;
            ImGui::SetNextItemWidth(width);
            if (InputStringOnLeave(I18N::Editor::ModelFile, "", part.model.file, typed) && typed.ends_with(".bmd"))
            {
                ChangePartModel(I18N::Editor::ModelFile, name,
                                [&typed](Data::Items::PartModel& changed) { changed.model.file = typed; });
            }
            ImGui::SetNextItemWidth(width);
            if (InputStringOnLeave(I18N::Editor::TextureFolders, "", JoinFolders(part.model.textureFolders), typed))
            {
                ChangePartModel(I18N::Editor::TextureFolders, name, [&typed](Data::Items::PartModel& changed)
                                { changed.model.textureFolders = SplitFolders(typed); });
            }
        }
        if (ImGui::CollapsingHeader(I18N::Editor::Meshes, ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (const ItemMeshRow& row : DescribeMeshes(Models[slot->modelType]))
            {
                ImGui::BulletText("%d  %s", row.mesh, row.texture.c_str());
            }
        }
        if (ImGui::CollapsingHeader(I18N::Editor::StyleAndEffect, ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SetNextItemWidth(width);
            const char* shown = part.renderStyle.empty() ? I18N::Editor::None : part.renderStyle.c_str();
            if (ImGui::BeginCombo(I18N::Editor::RenderStyle, shown))
            {
                std::optional<std::string> picked;
                if (ImGui::Selectable(I18N::Editor::None, part.renderStyle.empty()))
                {
                    picked = "";
                }
                for (const std::string_view style : Render::Items::Styles::GetNames())
                {
                    if (ImGui::Selectable(std::string(style).c_str(), style == part.renderStyle))
                    {
                        picked = std::string(style);
                    }
                }
                ImGui::EndCombo();
                if (picked)
                {
                    ChangePartModel(I18N::Editor::RenderStyle, name,
                                    [&picked](Data::Items::PartModel& changed) { changed.renderStyle = *picked; });
                }
            }
            ImGui::SameLine();
            ImGui::BeginDisabled(part.renderStyle.empty() || !Render::Items::Styles::Exists(part.renderStyle));
            if (ImGui::SmallButton(I18N::Editor::OpenInStyleEditor))
            {
                g_MuStyleEditorUI.ShowRenderStyle(part.renderStyle);
            }
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    ImGui::PopID();
}

void CMuItemModelUI::RenderPreview(const ItemModelDefinition* model, int itemType)
{
    // Some items are drawn by code with other models at some levels; the
    // Wizard's Ring only so, without a model entry.
    const bool drawnByCode = g_ItemDatabase.Find(itemType) != nullptr && IsDrawnByCodeAtSomeLevel(itemType);
    if ((model != nullptr || drawnByCode) &&
        ImGui::CollapsingHeader(I18N::Editor::Preview, ImGuiTreeNodeFlags_DefaultOpen))
    {
        m_preview.Render(itemType);
    }
}

void CMuItemModelUI::RenderModel(ItemModelDefinition* model, int itemType)
{
    const bool drawnByCode = g_ItemDatabase.Find(itemType) != nullptr && IsDrawnByCodeAtSomeLevel(itemType);
    if (drawnByCode)
    {
        RenderLevelModels(itemType);
    }
    if (ImGui::CollapsingHeader(I18N::Editor::ModelFile, ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (model != nullptr)
        {
            ImGui::TextUnformatted(model->file.c_str());
        }
        else
        {
            ImGui::TextDisabled("%s", I18N::Editor::NoModel);
        }
        m_filePicker.Render(itemType, model);
    }
    if (model == nullptr)
    {
        // What the inventory draws by code, without sliders to change it.
        if (drawnByCode && ImGui::CollapsingHeader(I18N::Editor::InTheInventory, ImGuiTreeNodeFlags_DefaultOpen))
        {
            m_inventoryGrid.Render(itemType, m_preview.GetLevel(), m_preview.IsExcellent(), m_preview.IsAncient());
        }
        return;
    }
    if (ImGui::CollapsingHeader(I18N::Editor::SharedModel, ImGuiTreeNodeFlags_DefaultOpen))
    {
        m_sharedForm.Render(*model, m_counts);
    }
    if (model->model.empty())
    {
        ModelForms::RenderOwnFile(m_draft, *model);
    }
    m_meshList.Render(itemType, *model, m_draft, m_preview.ShowsListedModel(itemType));
    if (ImGui::CollapsingHeader(I18N::Editor::InTheInventory, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderInventory(*model, itemType);
    }
    ModelForms::RenderGround(m_draft, *model);
    ModelForms::RenderGlow(m_draft, *model, m_counts, m_newGlowColor);
    if (ImGui::CollapsingHeader(I18N::Editor::StyleAndEffect, ImGuiTreeNodeFlags_DefaultOpen))
    {
        ModelForms::RenderLooks(m_draft, *model, m_counts);
    }
    RenderRemove(*model);
}

void CMuItemModelUI::RenderLevelModels(int itemType)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::DrawnByCode, ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    ImGui::TextDisabled("%s", I18N::Editor::TheGameChoosesTheseModelsByCodeTheyAreNotEditedHere);
    const int ownModel = Data::Items::ToModelSlot(itemType);
    for (const LevelModelRun& run : FindLevelModelRuns(itemType))
    {
        std::string levels = "+" + std::to_string(run.firstLevel);
        if (run.lastLevel != run.firstLevel)
        {
            levels += " - +" + std::to_string(run.lastLevel);
        }
        if (!run.modelType)
        {
            ImGui::BulletText("%s: %s", levels.c_str(), I18N::Editor::NotDrawn);
            continue;
        }
        std::string what = *run.modelType == ownModel
                               ? std::string(I18N::Editor::ItsModelEntry)
                               : Core::Text::ToUtf8(gLoadData.GetModelFile(*run.modelType).c_str());
        // Another item's model: name the item.
        if (*run.modelType != ownModel && Data::Items::IsItemModelSlot(*run.modelType))
        {
            const int other = Data::Items::ToItemType(*run.modelType);
            if (g_ItemDatabase.Find(other) != nullptr)
            {
                what += " (" + I18N::Format(I18N::Editor::TheModelOf0, {g_ItemDatabase.GetLogName(other)}) + ")";
            }
        }
        if (run.small)
        {
            what += std::string(", ") + I18N::Editor::DrawnSmall;
        }
        ImGui::BulletText("%s: %s", levels.c_str(), what.c_str());
    }
}

// The sliders, and beside them the item in an inventory grid.
void CMuItemModelUI::RenderInventory(ItemModelDefinition& model, int itemType)
{
    if (!ImGui::BeginTable("##Inventory", 2, ImGuiTableFlags_SizingFixedFit))
    {
        return;
    }
    ImGui::TableNextColumn();
    ModelForms::RenderInventory(m_draft, model.inventory);
    // A slider moved in this frame: the grid and the inventories drawn after
    // it show it.
    ShowInventoryDisplay(&model, itemType);
    ImGui::TableNextColumn();
    m_inventoryGrid.Render(itemType, m_preview.GetLevel(), m_preview.IsExcellent(), m_preview.IsAncient());
    ImGui::EndTable();
}

void CMuItemModelUI::BeforeFrame()
{
    m_preview.BeforeFrame();
    m_inventoryGrid.BeforeFrame();
    Render::Items::Display::ClearInventoryDisplayPreview();
}

// The draft's inventory display goes to the game's inventory drawing until the
// next frame, also before it is stored (a slider being dragged).
void CMuItemModelUI::ShowInventoryDisplay(const ItemModelDefinition* model, int itemType)
{
    if (model != nullptr && Data::Items::IsValidItemType(itemType))
    {
        Render::Items::Display::PreviewInventoryDisplay(itemType, model->inventory);
    }
}

void CMuItemModelUI::RenderRemove(const ItemModelDefinition& model)
{
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Button, RemoveColor);
    if (ImGui::Button(I18N::Editor::RemoveTheModelEntry))
    {
        ItemModelDefinition none;
        none.group = model.group;
        none.number = model.number;
        g_ItemEdits.SetItemModel(I18N::Editor::RemoveTheModelEntry, none);
    }
    ImGui::PopStyleColor();
}

#endif // _EDITOR
