#pragma once

namespace Engine::Items::Drops
{
// The model a dropped item is drawn with on the ground: its own, but for some items at some levels another one (the
// Box of Luck's kinds, a chocolate box opened at +1, the three Weapons of Archangel, the jewels and refine stones). The
// ground label's tag conditions read the item of this model.
int DrawnModel(int itemType, int level);
} // namespace Engine::Items::Drops
