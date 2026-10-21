#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemModelUI.h"

#include "ItemEdits.h"
#include "ItemLevelModels.h"
#include "ItemModelForms.h"
#include "ItemSelection.h"
#include "ItemToolWindow.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Core/Text/Utf8.h"
#include "Data/DataHandler/LoadData.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "Render/Items/ItemDisplay.h"
#include "imgui.h"

#include <optional>
#include <string>

using namespace MuEditor::Items;
using Data::Items::ItemModelDefinition;

namespace
{
constexpr ImVec4 RemoveColor{0.7f, 0.2f, 0.2f, 1.0f};

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
        ItemModelDefinition* model = m_draft.Begin(itemType, ToolWindow::GetDataVersion());
        ShowInventoryDisplay(model, itemType);
        // Also a number with a model but no item can be edited here.
        ToolWindow::RenderSelectedItem(itemType);
        if (Data::Items::IsValidItemType(itemType))
        {
            ImGui::PushID(itemType);
            RenderModel(model, itemType);
            ImGui::PopID();
        }
        // Outside the item's ID scope: the warning's list selects other items.
        m_sharedForm.RenderConfirm();
        m_draft.End();
    }
    ToolWindow::End();
}

void CMuItemModelUI::RenderModel(ItemModelDefinition* model, int itemType)
{
    // Some items are drawn by code with other models at some levels; the
    // Wizard's Ring only so, without a model entry.
    const bool drawnByCode = g_ItemDatabase.Find(itemType) != nullptr && IsDrawnByCodeAtSomeLevel(itemType);
    if ((model != nullptr || drawnByCode) &&
        ImGui::CollapsingHeader(I18N::Editor::Preview, ImGuiTreeNodeFlags_DefaultOpen))
    {
        m_preview.Render(itemType);
    }
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
    if (ImGui::CollapsingHeader(I18N::Editor::InTheInventory, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderInventory(*model, itemType);
    }
    ModelForms::RenderGround(m_draft, *model);
    ModelForms::RenderGlow(m_draft, *model, m_counts, m_newGlowColor);
    if (ImGui::CollapsingHeader(I18N::Editor::Looks, ImGuiTreeNodeFlags_DefaultOpen))
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
