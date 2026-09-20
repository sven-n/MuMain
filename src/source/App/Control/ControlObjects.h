// What the control socket knows about the objects around the character.
//
// This is the one place that reads the engine's object tables for the control
// layer, so the event ring and the protocol stay free of game globals.
#pragma once

#include "App/Control/ControlEvents.h"

#include <string>
#include <utility>

namespace App::Control
{
// Describes a character, monster, NPC or pet by its server-assigned key.
// An object the client does not know is reported with `known == false`,
// its id and kind `unknown`.
[[nodiscard]] Events::ObjectDescription DescribeGameObject(int key);

// The name the item tables give an item, empty for an empty slot (type -1;
// type 0 is a real item).
[[nodiscard]] std::wstring ItemDisplayName(int itemType, int itemLevel);

// The same for a dropped item, by the client's own slot in the drop table.
[[nodiscard]] std::wstring DropName(int itemSlot);

// The tile a dropped item lies on, converted from its world position;
// {-1, -1} for a slot outside the table.
[[nodiscard]] std::pair<int, int> DropTile(int itemSlot);
} // namespace App::Control
