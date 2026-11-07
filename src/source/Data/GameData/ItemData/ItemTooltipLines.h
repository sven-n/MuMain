#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <string>
#include <string_view>
#include <vector>

namespace Data::Items
{
// A line of an item's own that its tooltip writes, and the field the item file writes it at ("description[2]",
// "nameLines[0]", "tooltipLayoutChanges.add[1].lines[0]").
struct TooltipLineOf
{
    const ItemDescriptionLine* line;
    std::string field;
};

// The lines ForEachItemTooltipLine walks (the description lines, the name lines, the added lines), each with its
// field in the item file.
std::vector<TooltipLineOf> TooltipLinesOf(const ItemDefinition& definition);

// The field of a line's text in one language, as the item file writes it: "description[2].text" for English,
// "description[2].text.de" for a translation.
std::string LineTextField(const std::string& lineField, std::string_view locale);
} // namespace Data::Items
