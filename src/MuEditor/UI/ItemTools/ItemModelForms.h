#pragma once

#ifdef _EDITOR

#include "GlowColorDialog.h"
#include "ItemEditDraft.h"
#include "ItemLookUseCounts.h"

#include "Data/GameData/ItemData/ItemModelDefinition.h"

// The parts of the model tool that edit the item's own model values: the
// file's texture folders and none-blend meshes (for a file of its own), the
// inventory and ground display, the cloth flag, the material, the glow, and
// the render style and item effect picked from their lists with the items
// that use them.
namespace MuEditor::Items::ModelForms
{
using ModelDraft = ItemEditDraft<Data::Items::ItemModelDefinition>;

void RenderOwnFile(ModelDraft& draft, Data::Items::ItemModelDefinition& model);
// The inventory display's sliders (without a header).
void RenderInventory(ModelDraft& draft, Data::Items::ItemInventoryDisplay& inventory);
// The ground display, the cloth flag and the material.
void RenderGround(ModelDraft& draft, Data::Items::ItemModelDefinition& model);
// The glow; beside each of its colors a button that adds a new color to the
// list (CGlowColorDialog) and picks it, as one undo step.
void RenderGlow(ModelDraft& draft, Data::Items::ItemModelDefinition& model, ItemLookUseCounts& counts,
                CGlowColorDialog& newColor);
void RenderLooks(ModelDraft& draft, Data::Items::ItemModelDefinition& model, ItemLookUseCounts& counts);
} // namespace MuEditor::Items::ModelForms

#endif // _EDITOR
