#pragma once

#ifdef _EDITOR

#include <vector>

// Which code reads a monster's attacks, for the Monster Attacks tool. The
// handlers of SetPlayerAttack answer for the monsters they list, on their
// maps (monsters design, Q-MN13); the attacks of a monster that none answers
// for do nothing. This list follows the handlers' code (ZzzCharacter.cpp
// SetPlayerAttack, the GM*.cpp handlers): a change there belongs here too.
namespace MuEditor::Monsters::AttackReaders
{
// The handlers list monster numbers and answer on their maps, unless said.
enum class Reader
{
    // On any map.
    Hellas,
    // By model, on any map.
    BattleCastleGuards,
    HuntingGround,
    Aida,
    // On Crywolf and in Devil Square.
    CrywolfAndDevilSquare,
    Kanturu1st,
    // On Kanturu 2nd and 3rd.
    Kanturu2ndAnd3rd,
    Kanturu3rd,
    // In the barracks and the refuge.
    BalgassBarracks,
    IllusionTemple,
    // By model.
    SwampOfQuiet,
    // On Raklion and its boss map.
    Raklion,
    // On the four Empire Guardian maps.
    EmpireGuardian,
    EmpireGuardian2,
    EmpireGuardian3,
    EmpireGuardian4,
    // SetPlayerAttack, by model, before any handler.
    Traps,
    Bali,
};

// The readers of the monster's attacks; `modelSlot` is the slot of its model
// (the model-keyed readers), -1 when it has none.
std::vector<Reader> FindReaders(int number, int modelSlot);
} // namespace MuEditor::Monsters::AttackReaders

#endif // _EDITOR
