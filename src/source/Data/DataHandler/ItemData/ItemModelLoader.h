#pragma once

#include <string>

// Opens the item models of the item model database (Data/Items/Models) into
// the model slots MODEL_ITEM + item type, and loads their textures. The model
// data must be loaded first (CItemDataHandler::LoadModels).
namespace Data::Items::ModelLoader
{
// Opens the .bmd file of every item model.
void OpenModels();

// Loads the textures of every item model from its texture folders.
void OpenTextures();

// The message for the player about the missing model files and textures
// that OpenModels and OpenTextures found (all problems are already in the
// log), or an empty text when there are none. Called once the item data is
// loaded, so the message can name the items.
std::string TakeProblemMessage();
} // namespace Data::Items::ModelLoader
