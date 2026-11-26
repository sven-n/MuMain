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

// The items' links: a name of no definition (an error: the link is left out), two definitions whose options the
// same part of the server's item data turns on (an error naming the one left out), excellent options on wings and
// horns and wing options on horns (errors), wing options on another item that is no wing (a warning: its byte holds
// excellent options), and an ancient bonus or set options (a warning: the item sets give them, not the item's links).
void ValidateItemOptionLinks(std::span<const ItemDefinition> items, const ItemOptionDatabase& options,
                             std::vector<ItemDataIssue>& issues);

// A definition neither an item nor a rule of `options` links (a warning; without the rules, as in the game, only the
// items count), except the ancient bonus definitions the items' attackType picks.
void ValidateOptionDefinitionUse(std::span<const ItemDefinition> items, const ItemOptionDatabase& options,
                                 std::vector<ItemDataIssue>& issues);

// The option files as Save would write them (the editor's changes): each file is written and read back, so a problem
// only the reader finds stops it too, and the checks above and ValidateAncientBonusNames run on what was read. The
// problems name the files as "Options/<file>", "Options/OptionRules.json" and "Tooltips/OptionTexts.json".
void ValidateItemOptionFilesAsWritten(const ItemOptionFiles& files, std::vector<ItemDataIssue>& issues);

// An ancient bonus name of AncientBonusNames with no definition of type ancientBonus (a warning: the items with that
// attackType show and add no bonus).
void ValidateAncientBonusNames(const ItemOptionDatabase& options, std::vector<ItemDataIssue>& issues);
} // namespace Data::Items
