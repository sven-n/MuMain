#pragma once

#include "Data/GameData/EffectData/GlowColors.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "Data/GameData/ItemData/ItemTooltipLayouts.h"
#include "Data/GameData/ItemData/SharedItemModel.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Items
{
// Checks rules across items that a single file cannot check: unique ids,
// the required English name, and names that fit the LocalizedString format.
// Also the description texts: the English text, the language codes, the
// braces in every language, and the English text's placeholders in each
// translation. An entry without a name is only checked for its id. Appends
// the problems to `issues`.
void ValidateItems(std::span<const ItemDefinition> items, std::vector<ItemDataIssue>& issues);

// A translation of an item name or a description text in a language the game
// has no texts for is never shown: a warning for each. `uiLocales` are the
// game's languages (I18N::GetAvailableLocales).
void ValidateNameLocales(std::span<const ItemDefinition> items, std::span<const char* const> uiLocales,
                         std::vector<ItemDataIssue>& issues);

// Checks that no item has more than one model. Appends the problems to `issues`.
void ValidateItemModels(std::span<const ItemModelDefinition> models, std::vector<ItemDataIssue>& issues);

// The item files list every number the client uses: a model entry without an item entry (an
// item or an entry without a name) at its number is an error.
void ValidateModelsHaveItemEntries(std::span<const ItemDefinition> items, std::span<const ItemModelDefinition> models,
                                   std::vector<ItemDataIssue>& issues);

// The stat profiles of the items that exist against the item stat file
// (`statFile` names it in messages): a profile the file does not have is an
// error; a profile of the file that no item names, and an excellent bonus where
// the item's profile has no excellent term for its damage or magic power, are
// warnings.
void ValidateStatProfiles(std::span<const ItemDefinition> items, const ItemStatProfiles& profiles,
                          const std::string& statFile, std::vector<ItemDataIssue>& issues);

// The placeholders of the items' descriptions, written in place or in a shared text of `tooltips`, against the item
// stat file (`statFile` names it in messages). A placeholder is a built-in value, "table:" and a table of the file, one
// the line's "values" names, or one of the item's "levelValues"; another name is an error, with the nearest known name
// when it looks like a typo. The line's values and the item's level values are its own names: they win over a
// built-in value of the same name. A line's "values" names a built-in value or a table; a line that shows a level
// value is shown only at levels that have one. A level value no line shows is a
// warning. The braces and the translations are checked by ValidateItems.
void ValidateDescriptionPlaceholders(std::span<const ItemDefinition> items, const ItemStatProfiles& profiles,
                                     const ItemTooltipFile& tooltips, const std::string& statFile,
                                     std::vector<ItemDataIssue>& issues);

// The shared texts ("@name") of the items' descriptions and level values against the tooltip file (`tooltipFile`
// names it in messages): a text the file does not have is an error, with the nearest name when it looks like a typo
// (their placeholders are checked by ValidateDescriptionPlaceholders). A level value has no placeholders. A shared text
// that neither a layout nor a description uses is a warning.
void ValidateDescriptionTexts(std::span<const ItemDefinition> items, const ItemTooltipFile& tooltips,
                              const std::string& tooltipFile, std::vector<ItemDataIssue>& issues);

// The tables of the item stat file the items use (`statFile` names it in messages), in their description and name
// lines' texts (written in place or shared in `tooltips`) and "values". A table that neither a stat nor a description
// names is a warning. So is a wing (an item with a wing tier) whose description lines are not those of most of the
// other wings of its tier, cape tag and group (or, alone there, of its tier and cape tag in the other groups), and a
// tie among them: the tooltip code chose the wings' lines by the tier, so a new or re-tiered wing that misses them,
// keeps another tier's, or loses or gains a line, is pointed out.
void ValidateTableUses(std::span<const ItemDefinition> items, const ItemStatProfiles& profiles,
                       const ItemTooltipFile& tooltips, const std::string& statFile,
                       std::vector<ItemDataIssue>& issues);

// Gives the model entries that name a shared model ("model") its file,
// texture folders and none-blend meshes. A name that is not a shared model
// and a shared model defined twice are errors; a shared model that no item
// uses is a warning. `sharedModelsSource` names the shared model file.
void ApplySharedItemModels(std::vector<ItemModelDefinition>& models, std::span<const SharedItemModel> sharedModels,
                           const std::string& sharedModelsSource, std::vector<ItemDataIssue>& issues);

// Checks that the glow colors of the models, and the default ones, are in
// the glow color list. Appends the problems to `issues`.
void ValidateItemModelGlowColors(std::span<const ItemModelDefinition> models,
                                 std::span<const Effects::GlowColor> colors, std::vector<ItemDataIssue>& issues);

// Whether a look of this name exists; the looks are drawing code
// (Render::Items::Styles::Exists, Render::Items::ItemEffects::Exists).
using LookNameExists = bool (*)(std::string_view name);

// A render style or item effect that no look of that name has: a warning for
// each, since the model is then drawn plainly (the model loader reports the
// same when it opens the models).
void ValidateItemModelLooks(std::span<const ItemModelDefinition> models, LookNameExists renderStyleExists,
                            LookNameExists itemEffectExists, std::vector<ItemDataIssue>& issues);

// A .bmd file that several model entries name themselves instead of through
// a shared model: a warning for each of them, since each opens the file again
// (a shared model opens it once).
void ValidateDirectlySharedModelFiles(std::span<const ItemModelDefinition> models, std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
