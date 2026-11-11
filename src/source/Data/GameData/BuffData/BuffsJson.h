#pragma once

#include "Data/GameData/BuffData/BuffDefinition.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Data/Buffs/Buffs.json (buffs design, B1, B10, B12):
//
//   { "formatVersion": 1,
//     "buffs": [
//       { "number": 6, "name": "infinityArrow", "kind": "buff", "group": 6,
//         "displayName": {"en": "Infinity Arrow", "es": "Flecha Infinita", ...},
//         "description": {"en": "Arrows are not consumed.", ...},
//         "family": "infinityArrow", "cancelSkill": 77 },
//       { "number": 40, "name": "sealOfAscension", ..., "item": {"group": 13, "number": 43} } ] }
//
// "description" lines are separated by "\n". "item", "family", "hidden" and "cancelSkill" are left out when a buff has
// none, and "tier" when it is 0.
namespace Data::Buffs
{
constexpr int BuffsFormatVersion = 1;

// Reads the entries into `buffs`. An entry with a fatal problem is left out and its problem added to `issues`, with the
// field "buffs[i].key"; unknown fields are warnings. `entries`, if given, gets each read buff's index in the file.
void ReadBuffsJson(std::string_view text, const std::string& source, std::vector<BuffDefinition>& buffs,
                   std::vector<Items::ItemDataIssue>& issues, std::vector<size_t>* entries = nullptr);

// The file's text, the entries in their order.
std::string WriteBuffsJson(std::span<const BuffDefinition> buffs);

// What the reader cannot see in one entry: a number or a name used twice, two buffs of one family with the same tier
// (fatal: which would show?), a family of buffs and debuffs. The problems name buffs[i] by `entries` (the file's
// indices from ReadBuffsJson), or by their position in `buffs` without them.
void ValidateBuffs(std::span<const BuffDefinition> buffs, std::span<const size_t> entries, const std::string& source,
                   std::vector<Items::ItemDataIssue>& issues);

// A buff whose item the item list does not have, as an error: the buff gets no duration or values from it.
// `itemExists` tells whether the list has the item of that group and number. `entries` as for ValidateBuffs.
void ValidateBuffItems(std::span<const BuffDefinition> buffs, std::span<const size_t> entries,
                       const std::function<bool(int group, int number)>& itemExists, const std::string& source,
                       std::vector<Items::ItemDataIssue>& issues);

// "buffs[2].displayName", as the problems name a field of an entry.
std::string BuffField(size_t entry, std::string_view key);
} // namespace Data::Buffs
