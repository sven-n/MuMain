#pragma once

// Opens the item models of the item model database (Data/Items/Models) into
// the model slots MODEL_ITEM + item type, and loads their textures. The model
// data must be loaded first (CItemDataHandler::LoadModels).
namespace Data::Items::ModelLoader
{
// Opens the .bmd file of every item model.
void OpenModels();

// Loads the textures of every item model from its texture folders.
void OpenTextures();

// Logs the problems OpenModels and OpenTextures found (missing model files
// and textures) and shows the errors to the player. Called once the item
// data is loaded, so the messages can name the items.
void ReportProblems();
} // namespace Data::Items::ModelLoader
