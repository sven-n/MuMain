#pragma once

namespace GameLogic::Items::TransformationRing
{
// The name of the monster a transformation ring of this level turns into, from
// the monster list; nullptr for a level without a form and for a form the list
// has no entry for.
const wchar_t* MonsterName(int ringLevel);
} // namespace GameLogic::Items::TransformationRing
