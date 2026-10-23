#pragma once

#include "Engine/Object/ZzzInventory.h"
#include "UI/Inventory/ItemMng.h"

#include <ostream>
#include <vector>

// An item as the server would send it: the fields not set are 0, the durability is full.
ItemCreationParams FullItemParameters(int group, int number);

// The information tooltip lines of such an item, for the hero as he is (BuildItemTooltipLines).
std::vector<ItemTooltipLine> TooltipOfParameters(const ItemCreationParams& parameters);

// Writes the tooltip lines of every item (of `onlyGroup` only, when it is 0 or more) at every level 0 to 15, as
// plain, full (luck, skill, the +4 option, every excellent option), ancient (set 1), Jewel of Harmony (option 1 at
// level 13), count (a durability of 3) and timed (expired) items: a line "<group>/<number> <variant> +<level>", then
// one line "  <color> [bold ]<text>" per tooltip line ("<blank>" for a blank one). Two snapshots compare with any diff
// tool, e.g. before and after a change of the tooltip code. Returns the number of tooltips written.
int WriteItemTooltipSnapshot(std::ostream& out, int onlyGroup = -1);
