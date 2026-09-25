#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"

#include <span>
#include <vector>

namespace Data::Items
{
// Checks rules across items that a single file cannot check: unique ids,
// the required English name, and names that fit the LocalizedString format.
// Appends the problems to `issues`.
void ValidateItems(std::span<const ItemDefinition> items, std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
