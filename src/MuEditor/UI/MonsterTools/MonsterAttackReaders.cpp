#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterAttackReaders.h"

#include "Core/Globals/_enum.h"

#include <algorithm>
#include <array>
#include <initializer_list>

namespace MuEditor::Monsters::AttackReaders
{
namespace
{
// The trap models SetPlayerAttack knows by their object type.
constexpr int TrapModels[] = {39, 40, 51};

struct NumberReader
{
    Reader reader;
    std::initializer_list<int> numbers;
};

// The map code that runs for these monster numbers.
const std::array<NumberReader, 6> NumberReaders = {{
    {Reader::Kanturu3rd, {MONSTER_MAYA}},
    {Reader::Raklion, {MONSTER_SELUPAN}},
    {Reader::EmpireGuardian, {MONSTER_RAYMOND, MONSTER_LUCAS, MONSTER_FRED, MONSTER_DEVIL_LORD}},
    {Reader::EmpireGuardian2, {MONSTER_HAMMERIZE}},
    {Reader::EmpireGuardian3, {MONSTER_DUAL_BERSERKER}},
    {Reader::EmpireGuardian4, {MONSTER_GAYION_THE_GLADIATOR, MONSTER_JERRY}},
}};

bool Contains(std::initializer_list<int> values, int value)
{
    return std::find(values.begin(), values.end(), value) != values.end();
}

template <std::size_t Count> bool Contains(const int (&values)[Count], int value)
{
    return std::find(std::begin(values), std::end(values), value) != std::end(values);
}
} // namespace

std::vector<Reader> FindReaders(int number, int modelSlot)
{
    std::vector<Reader> readers;
    // SetPlayerAttack plays the traps' effects before any map code.
    if (Contains(TrapModels, modelSlot))
    {
        readers.push_back(Reader::Traps);
        return readers;
    }
    for (const NumberReader& entry : NumberReaders)
    {
        if (Contains(entry.numbers, number))
        {
            readers.push_back(entry.reader);
        }
    }
    if (modelSlot == MODEL_BATTLE_GUARD1 || modelSlot == MODEL_BATTLE_GUARD2)
    {
        readers.push_back(Reader::BattleCastleGuards);
    }
    // In the order SetPlayerAttack runs them.
    std::sort(readers.begin(), readers.end());
    return readers;
}
} // namespace MuEditor::Monsters::AttackReaders

#endif // _EDITOR
