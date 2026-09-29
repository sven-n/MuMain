#pragma once

#include <string>
#include <string_view>

// Opens the item models of the item model database (Data/Items/Models) into
// the model slots MODEL_ITEM + item type, and loads their textures. The model
// data must be loaded first (CItemDataHandler::LoadModels).
namespace Data::Items::ModelLoader
{
// Whether a render style of this name exists (the render styles are drawing
// code, Render::Items::Styles::Exists).
using RenderStyleExists = bool (*)(std::string_view name);

// Opens the .bmd file of every item model; `renderStyleExists` checks the
// "renderStyle" names.
void OpenModels(RenderStyleExists renderStyleExists);

// Loads the textures of every item model from its texture folders.
void OpenTextures();

// The message for the player about the missing model files and textures
// that OpenModels and OpenTextures found (all problems are already in the
// log), or an empty text when there are none. Called once the item data is
// loaded, so the message can name the items.
std::string TakeProblemMessage();
} // namespace Data::Items::ModelLoader
