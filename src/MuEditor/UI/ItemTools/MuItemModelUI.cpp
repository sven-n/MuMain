#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemModelUI.h"

#include "ItemEdits.h"
#include "ItemModelForms.h"
#include "ItemSelection.h"
#include "ItemToolWindow.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
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
        ToolWindow::RenderUndoRedo();
        const int itemType = g_ItemSelection.GetItemType();
        ItemModelDefinition* model = m_draft.Begin(itemType, ToolWindow::GetDataVersion());
        ShowInventoryDisplay(model, itemType);
        ImGui::SameLine();
        // Also a number with a model but no item can be edited here.
        ToolWindow::RenderSelectedItem(itemType);
        if (Data::Items::IsValidItemType(itemType))
        {
            ImGui::PushID(itemType);
            RenderModel(model, itemType);
            ImGui::PopID();
        }
        m_draft.End();
    }
    ToolWindow::End();
}

void CMuItemModelUI::RenderModel(ItemModelDefinition* model, int itemType)
{
    if (model != nullptr && ImGui::CollapsingHeader(I18N::Editor::Preview, ImGuiTreeNodeFlags_DefaultOpen))
    {
        m_preview.Render(itemType);
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
    ModelForms::RenderDisplay(m_draft, *model);
    // A slider moved in this frame: the inventories drawn after it show it.
    ShowInventoryDisplay(model, itemType);
    ModelForms::RenderGlow(m_draft, *model, m_counts);
    if (ImGui::CollapsingHeader(I18N::Editor::Looks, ImGuiTreeNodeFlags_DefaultOpen))
    {
        ModelForms::RenderLooks(m_draft, *model, m_counts);
    }
    RenderRemove(*model);
}

void CMuItemModelUI::BeforeFrame()
{
    m_preview.BeforeFrame();
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
