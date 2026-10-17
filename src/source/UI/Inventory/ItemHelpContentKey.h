#pragma once

#include "Engine/Object/ZzzInventory.h"

namespace mu::ui::window
{
// What the item help window's lines and table are built from: the item and its data (the item
// editor and a reload raise the item database's version), the language, and the hero's class,
// level and stats. The window builds them again only when this changes.
struct ItemHelpContentKey
{
    int itemType = -1;
    int itemDataVersion = 0;
    int localeIndex = 0;
    CLASS_TYPE heroClass{};
    ItemInfoHeroStats heroStats{};

    bool operator==(const ItemHelpContentKey&) const = default;
};

ItemHelpContentKey GetItemHelpContentKey(int itemType);
} // namespace mu::ui::window
