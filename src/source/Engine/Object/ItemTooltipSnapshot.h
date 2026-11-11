#pragma once

#include "Engine/Object/ZzzInventory.h"
#include "UI/Inventory/ItemMng.h"

#include <functional>
#include <ostream>
#include <string>
#include <vector>

namespace Engine::Items::TooltipSnapshot
{
// An item as the server would send it: the fields not set are 0, the durability is the item data's base durability
// (the snapshot is recorded with it; FullDurability gives the full one).
ItemCreationParams FullItemParameters(int group, int number);

// The full durability of the item made from `parameters`, as CalcMaxDurability gives it (it grows with the level, an
// excellent or ancient option raises it, and a staff, stick or book starts from its magic durability), at most 255:
// what the item's durability byte holds.
BYTE FullDurability(const ItemCreationParams& parameters);

// The information tooltip lines of such an item, for the hero as he is (BuildItemTooltipLines).
std::vector<ItemTooltipLine> TooltipOfParameters(const ItemCreationParams& parameters);

// The text of one item's tooltips in a snapshot, every variant and level (the lines WriteItemTooltipSnapshot writes
// for it).
using ItemSnapshot = std::function<void(int group, int number, const std::string& text)>;

// Writes the tooltip lines of every item (of `onlyGroup` only, when it is 0 or more) at every level 0 to 15, as
// plain, full (luck, skill, the +4 option, every excellent option), ancient (set 1) and Jewel of Harmony (option 1 at
// level 13) items; with luck, skill, the +1 option or one excellent option (the first) alone; at durability 1 and 0;
// and expired: a line "<group>/<number> <variant> +<level>", then one line "  <color> [bold ]<text>" per tooltip
// line ("  <blank>", without color or style, for a blank one). Two snapshots compare with any diff tool, e.g. before
// and after a change of the tooltip code. Returns the number of tooltips written.
int WriteItemTooltipSnapshot(std::ostream& out, int onlyGroup = -1);

// The same snapshot handed to `item` one item at a time, so a caller needs no more than one item's text at once.
// Returns the number of tooltips.
int VisitItemTooltipSnapshots(const ItemSnapshot& item, int onlyGroup = -1);
} // namespace Engine::Items::TooltipSnapshot
