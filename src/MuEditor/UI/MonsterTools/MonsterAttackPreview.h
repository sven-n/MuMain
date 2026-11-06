#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterAttacks.h"

#include <cstdint>

// What the Monster Attacks tool's Play next plays: the animation of a
// monster's next normal attack, as the game chooses it.
namespace MuEditor::Monsters::AttackPreview
{
// The counts the game keeps for one monster from its creation and its normal
// attacks change: the turn of its rotation (CHARACTER::AttackRotation) and
// CHARACTER::SwordCount.
struct Counts
{
    int rotation = 0;
    std::uint8_t swordCount = 0;
};

// The animation of the next attack without a monster skill or player skill
// entry: `otherwise`, the rotation's turn, the generic pattern's next one or
// the draw's pick; NoAttackAnimation for none. The counts go on as the
// game's do (MonsterAttackPlay's Play, and SetPlayerAttack counts every
// attack).
int NextOtherwise(const Data::Monsters::MonsterAttacks& attacks, Counts& counts);
} // namespace MuEditor::Monsters::AttackPreview

#endif // _EDITOR
