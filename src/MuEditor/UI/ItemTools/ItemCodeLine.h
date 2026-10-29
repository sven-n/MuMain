#pragma once

#ifdef _EDITOR

#include "ItemCodeReferences.h"
#include "UI/Common/CodeReferenceScan.h"

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Reading one line of C++ code, with its comments and string literals
// blanked, for the items it names (see ItemCodeReferences).
namespace MuEditor::Items
{
// The ITEM_ and MODEL_ constants of the items, by name, with their item types.
using ItemConstants = std::map<std::string, int, std::less<>>;

// An item a line names, and how; the number is always a valid item type.
using ItemMention = CodeReferenceScan::Mention;

// The constant a line of _enum.h defines for an item, written as
// ITEM_KRIS = ITEM_SWORD + 0, (MODEL_ alike): its name, which points into
// `code`, and its item type.
std::optional<std::pair<std::string_view, int>> ReadItemConstant(std::string_view code);

// The items the line names, each once, by the first kind that fits (Name,
// RawId, Range); the rules are those of ItemCodeReferences::Find.
std::vector<ItemMention> ReadItemMentions(std::string_view code, const ItemConstants& constants);
} // namespace MuEditor::Items

#endif // _EDITOR
