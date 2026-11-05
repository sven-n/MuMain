#pragma once

namespace GameLogic::Items::NameParts
{
// The name of the monster with this number as a part of an item name: in the
// UI language, or in English while Translate item names is off, like the rest
// of the item name. nullptr for a number the monster list has no entry for.
const wchar_t* MonsterName(int monsterNumber);
} // namespace GameLogic::Items::NameParts
