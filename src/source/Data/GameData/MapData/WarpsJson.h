#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MapData/Warp.h"

#include <functional>
#include <string>
#include <string_view>
#include <vector>

// Data/Maps/Warps.json:
//
//   { "formatVersion": 1,
//     "classLevelDiscount": [ {"class": "darkLord", "numerator": 2, "denominator": 3} ],
//     "warps": [
//       { "index": 23, "name": {"en": "Icarus", "es": "Ícaro"}, "command": "Icarus",
//         "level": 170, "zen": 10000, "gate": 63,
//         "needsOneOf": [ {"tag": "flying"}, {"wings": true} ],
//         "notWith": [ {"item": {"group": 13, "number": 2}},
//                      {"tag": "transformationRing", "except": {"tag": "pandaOrSkeleton"}} ],
//         "blockedMessage": "YouCanEnterIcarusOnlyWithWingsDinorantFenrirr" } ] }
//
// The rules of a warp are optional: "noClassDiscount", "minMasterLevel",
// "minResets", "pvpServersOnly", "gensOnly", "needsOneOf", "notWith" and
// "blockedMessage" (see Warp.h).
namespace Data::Maps
{
constexpr int WarpsFormatVersion = 1;

// The largest values the reader takes: an index (the warp request sends it in
// two bytes), a level, master level or gate, and a numerator or denominator of a
// class level discount (zen and resets go up to the largest int).
constexpr int MaxWarpIndex = 65535;
constexpr int MaxWarpLevel = 65535;
constexpr int MaxWarpGate = 65535;
constexpr int MaxDiscountPart = 1000;

// Reads the warps into `list`. A warp with a fatal problem is left out and its
// problem added to `issues`, with the field "warps[i].key"; unknown fields, item
// tags and message keys are warnings.
void ReadWarpsJson(std::string_view text, const std::string& source, WarpList& list,
                   std::vector<Items::ItemDataIssue>& issues);

// Sets the tag values of the item rules from their names (WarpItemMatch), so
// the rules match without looking names up; the reader does it, and the game
// when it takes a list.
void ResolveItemTags(WarpList& list);

// The file's text, the warps in their order, each on its own line.
std::string WriteWarpsJson(const WarpList& list);

// What the reader cannot see in one warp: an index two warps have (fatal: the
// game keeps one entry per index), and as warnings a name longer than the game
// holds (it is cut), a name or /move name with a space (/move takes one word),
// a class level discount that asks more level or none, an item rule entry
// twice in its list or with its own tag as "except" (it matches nothing) and a
// blocked message of a warp without equipment rules (never shown).
void ValidateWarps(const WarpList& list, const std::string& source, std::vector<Items::ItemDataIssue>& issues);

// The items the rules name that the item list does not have (`itemExists`
// false): errors, such an entry matches no item.
void ValidateWarpItems(const WarpList& list, const std::function<bool(int group, int number)>& itemExists,
                       const std::string& source, std::vector<Items::ItemDataIssue>& issues);

// "warps[2].level", as the problems name a field of a warp.
std::string WarpField(size_t warp, std::string_view key);
} // namespace Data::Maps
