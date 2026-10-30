#pragma once

#ifdef _EDITOR

#include <vector>

// The map code that runs with a monster's attacks, for the Monster Attacks
// tool. The attacks play on every map (monsters design, Q-MN64); what stays
// code is no choice of animation: the traps' effects, the Battle Castle
// guards' attack state, Maya's scene, Selupan's height, sounds and effects,
// and the Empire Guardian bosses' rage. This list follows that code
// (ZzzCharacter.cpp SetPlayerAttack, the GM*.cpp handlers): a change there
// belongs here too.
namespace MuEditor::Monsters::AttackReaders
{
enum class Reader
{
    // By model, on any map: the guards' attack state.
    BattleCastleGuards,
    // Maya's scene, on Kanturu 3rd.
    Kanturu3rd,
    // Selupan, on Raklion and its boss map.
    Raklion,
    // The bosses' rage, on the four Empire Guardian maps.
    EmpireGuardian,
    EmpireGuardian2,
    EmpireGuardian3,
    EmpireGuardian4,
    // SetPlayerAttack, by model, before any map code: their effects.
    Traps,
};

// The map code that runs with the monster's attacks; `modelSlot` is the slot
// of its model (the model-keyed code), -1 when it has none.
std::vector<Reader> FindReaders(int number, int modelSlot);
} // namespace MuEditor::Monsters::AttackReaders

#endif // _EDITOR
