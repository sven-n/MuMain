#pragma once

namespace UI::Inventory
{
// Whether the trade window remembers an item the partner offers, so that it can warn when the partner takes
// it out and puts in a worse copy (CTrade::BackUpYourInven, CTrade::AlertYourTradeInven): the pets up to the
// Dark Horse, the jewels and packed jewels, wings and capes, and weapons and armor at +5 or with excellent
// options. Potions, boxes, tickets and the other items of groups 13 to 15 aren't remembered.
[[nodiscard]] bool IsTrackedForChangeWarning(int itemType, int level, int excellentFlags, bool compiledGem);
} // namespace UI::Inventory
