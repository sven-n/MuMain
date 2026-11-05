#pragma once

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemTooltipConditions.h"

#include <functional>
#include <string>
#include <vector>

// Reading a "when" list of the item files and the tooltip file (apart from ItemTooltipConditions.h, which many headers
// include, so they don't take in the JSON library).
namespace Data::Items
{
// Reads `json`, a list of conditions as the files write them (ReadTooltipCondition), into `conditions`. Each problem
// goes to `addError` with its field: the list's, or an entry's ("<field>[2]"); the entries that read are kept.
// `example` shows a list in the error for a `json` that is no list.
void ReadTooltipConditions(const Json::OrderedJson& json, const std::string& field, std::string_view example,
                           std::vector<TooltipCondition>& conditions,
                           const std::function<void(const std::string& field, const std::string& message)>& addError);
} // namespace Data::Items
