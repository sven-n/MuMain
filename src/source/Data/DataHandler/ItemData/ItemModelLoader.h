#pragma once

#include <string>
#include <string_view>

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

// The message for the player about the missing model files and textures
// that OpenModels and OpenTextures found (all problems are already in the
// log), or an empty text when there are none. Called once the item data is
// loaded, so the message can name the items.
std::string TakeProblemMessage();
} // namespace Data::Items::ModelLoader
