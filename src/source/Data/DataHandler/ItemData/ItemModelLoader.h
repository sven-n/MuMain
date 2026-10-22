#pragma once

#include "Data/DataHandler/ItemData/ItemModelProblem.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// Opens the item models of the item model database (Data/Items/Models) into
// the model slots of their items (ToModelSlot), and loads their textures. The
// model data must be loaded first (CItemDataHandler::LoadModels).
namespace Data::Items::ModelLoader
{
// Whether a look of this name exists.
using LookExists = bool (*)(std::string_view name);

// No look exists: a check that is left out reports all names.
inline bool NoLookExists(std::string_view)
{
    return false;
}

// The looks that exist; they are drawing code (Render::Items::Styles::Exists,
// Render::Items::ItemEffects::Exists).
struct LookNames
{
    LookExists renderStyle = NoLookExists;
    LookExists itemEffect = NoLookExists;
};

// Opens the .bmd file of every item model; `lookNames` checks the names of
// the "renderStyle" and "itemEffect" values. The file of a shared model is
// opened once, by its first item; the others use the data of that slot.
void OpenModels(const LookNames& lookNames);

// Loads the textures of every item model from its texture folders (those of
// a shared model once).
void OpenTextures();

// The item editor moved two items, and with them their models
// (ItemModelDatabase::Swap): their model slots swap the opened data
// (CLoadData::SwapModels) and the material values of the models. Nothing
// happens before the game has its model slots.
void SwapOpenedModels(int firstItemType, int secondItemType);

// Opens the models of these items again into their model slots, with their
// textures, as the item model database has them now; the slot of an item
// without a model is emptied. Of the items of one shared model the first
// opens the file and the others share its data. For the item editor, which
// changes model files; the problems are returned, not kept for
// TakeProblemMessage. Nothing happens before the game has its model slots.
std::vector<ItemModelProblem> ReopenModels(std::span<const int> itemTypes, const LookNames& lookNames);

// The message for the player about the missing model files and textures
// that OpenModels and OpenTextures found (all problems are already in the
// log), or an empty text when there are none. Called once the item data is
// loaded, so the message can name the items.
std::string TakeProblemMessage();
} // namespace Data::Items::ModelLoader
