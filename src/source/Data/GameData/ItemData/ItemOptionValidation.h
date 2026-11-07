#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemOptionDatabase.h"

#include <span>
#include <string>
#include <vector>

// The checks of the option files across files: the definitions (Data/Items/Options) with their texts
// (Data/Items/Tooltips/OptionTexts.json), the option rules, and the items' links. A file's own checks are its reader's.
namespace Data::Items
{
// A definition name used twice, also in two files (fatal: a link would name either); an option whose attribute the
// client has no effect for (a warning: its line is shown, it adds nothing); an option naming a text the texts lack
// (an error: the line is left out); a text no option names (a warning). `textsSource` names the texts' file.
void ValidateItemOptions(std::span<const ItemOptionDefinition> definitions, const ItemOptionTexts& texts,
                         const std::string& textsSource, std::vector<ItemDataIssue>& issues);

// A rule naming no definition (fatal: the lists the editor writes from it would name it). `source` names the rules'
// file.
void ValidateOptionRules(std::span<const OptionRule> rules, std::span<const ItemOptionDefinition> definitions,
                         const std::string& source, std::vector<ItemDataIssue>& issues);

// The items' links: a name of no definition (an error: the link is left out), and two definitions whose options the
// same part of the server's item data turns on (an error: the later one is left out).
void ValidateItemOptionLinks(std::span<const ItemDefinition> items, const ItemOptionDatabase& options,
                             std::vector<ItemDataIssue>& issues);

// A definition neither an item nor a rule of `options` links (a warning; without the rules, as in the game, only the
// items count).
void ValidateOptionDefinitionUse(std::span<const ItemDefinition> items, const ItemOptionDatabase& options,
                                 std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
