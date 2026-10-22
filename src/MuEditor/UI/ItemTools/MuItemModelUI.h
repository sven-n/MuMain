#pragma once

#ifdef _EDITOR

#include "GlowColorDialog.h"
#include "ItemEditDraft.h"
#include "ItemInventoryGridPreview.h"
#include "ItemLookUseCounts.h"
#include "ItemModelFilePicker.h"
#include "ItemModelPreview.h"
#include "ItemSharedModelForm.h"

#include "Data/GameData/ItemData/ItemModelDefinition.h"

// The item model and visuals tool (section 9): the selected item's model as
// it is opened now, its model file (picked from Data/Item and Data/Player) or
// shared model, texture folders, inventory display (beside the item in an
// inventory grid) and ground display, cloth,
// material, glow, and the render style and item effect picked from their
// lists, each with the other items that use it (D24, D26). A changed file is
// opened again before the next frame.
class CMuItemModelUI
{
public:
    static CMuItemModelUI& GetInstance();

    void Render(bool* open);

    // Between frames: the preview's texture can only be released then, and
    // the inventory display shown before it is stored ends.
    void BeforeFrame();

private:
    CMuItemModelUI();

    void RenderModel(Data::Items::ItemModelDefinition* model, int itemType);
    void RenderInventory(Data::Items::ItemModelDefinition& model, int itemType);
    // The models the game draws the item with by code at some levels.
    void RenderLevelModels(int itemType);
    void RenderRemove(const Data::Items::ItemModelDefinition& model);
    void ShowInventoryDisplay(const Data::Items::ItemModelDefinition* model, int itemType);

    MuEditor::Items::ItemEditDraft<Data::Items::ItemModelDefinition> m_draft;
    MuEditor::Items::CItemModelPreview m_preview;
    MuEditor::Items::CItemInventoryGridPreview m_inventoryGrid;
    MuEditor::Items::CItemModelFilePicker m_filePicker;
    MuEditor::Items::CItemSharedModelForm m_sharedForm;
    MuEditor::Items::ItemLookUseCounts m_counts;
    MuEditor::Items::CGlowColorDialog m_newGlowColor;
};

#define g_MuItemModelUI CMuItemModelUI::GetInstance()

#endif // _EDITOR
