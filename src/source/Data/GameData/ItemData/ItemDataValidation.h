#pragma once

#include "Data/GameData/EffectData/GlowColors.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"

#include <span>
#include <vector>

namespace Data::Items
{
// Checks rules across items that a single file cannot check: unique ids,
// the required English name, and names that fit the LocalizedString format.
// Appends the problems to `issues`.
void ValidateItems(std::span<const ItemDefinition> items, std::vector<ItemDataIssue>& issues);

// Checks that no item has more than one model. Appends the problems to `issues`.
void ValidateItemModels(std::span<const ItemModelDefinition> models, std::vector<ItemDataIssue>& issues);

// Checks that the glow colors of the models, and the default ones, are in
// the glow color list. Appends the problems to `issues`.
void ValidateItemModelGlowColors(std::span<const ItemModelDefinition> models,
                                 std::span<const Effects::GlowColor> colors, std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
