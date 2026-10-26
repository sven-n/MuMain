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

// The handlers that list monster numbers, as their switches do.
const std::array<NumberReader, 14> NumberReaders = {{
    {Reader::Hellas,
     {MONSTER_DEATH_CENTURION_1,
      MONSTER_DEATH_CENTURION_2,
      MONSTER_DEATH_CENTURION_3,
      MONSTER_DEATH_CENTURION_4,
      MONSTER_DEATH_CENTURION_5,
      MONSTER_DEATH_CENTURION_6,
      MONSTER_AEGIS_1,
      MONSTER_AEGIS_2,
      MONSTER_AEGIS_3,
      MONSTER_AEGIS_4,
      MONSTER_AEGIS_5,
      MONSTER_AEGIS_6,
      MONSTER_ROGUE_CENTURION_1,
      MONSTER_ROGUE_CENTURION_2,
      MONSTER_ROGUE_CENTURION_3,
      MONSTER_ROGUE_CENTURION_4,
      MONSTER_ROGUE_CENTURION_5,
      MONSTER_ROGUE_CENTURION_6,
      MONSTER_NECRON_1,
      MONSTER_NECRON_2,
      MONSTER_NECRON_3,
      MONSTER_NECRON_4,
      MONSTER_NECRON_5,
      MONSTER_NECRON_6,
      MONSTER_SCHRIKER_1,
      MONSTER_SCHRIKER_2,
      MONSTER_SCHRIKER_3,
      MONSTER_SCHRIKER_4,
      MONSTER_SCHRIKER_5,
      MONSTER_SCHRIKER_6,
      MONSTER_ILLUSION_OF_KUNDUN_1,
      MONSTER_ILLUSION_OF_KUNDUN_2,
      MONSTER_ILLUSION_OF_KUNDUN_3,
      MONSTER_ILLUSION_OF_KUNDUN_4,
      MONSTER_ILLUSION_OF_KUNDUN_5,
      MONSTER_ILLUSION_OF_KUNDUN_7}},
    {Reader::HuntingGround, {MONSTER_FIRE_GOLEM}},
    {Reader::Aida,
     {MONSTER_WITCH_QUEEN, MONSTER_BLUE_GOLEM, MONSTER_HELL_MAINE, MONSTER_BLOODY_ORC, MONSTER_BLOODY_DEATH_RIDER,
      MONSTER_BLOODY_GOLEM, MONSTER_BLOODY_WITCH_QUEEN}},
    {Reader::CrywolfAndDevilSquare,
     {MONSTER_DARK_ELF, MONSTER_DARKELF, MONSTER_BALRAM, MONSTER_DEATH_SPIRIT, MONSTER_SORAM, MONSTER_BALGASS}},
    {Reader::Kanturu1st,
     {MONSTER_BERSERKER, MONSTER_GIGANTIS, MONSTER_GENOCIDER, MONSTER_SPLINTER_WOLF, MONSTER_IRON_RIDER,
      MONSTER_GIGANTIS_WARRIOR, MONSTER_KENTAUROS, MONSTER_BERSERKER_WARRIOR, MONSTER_KENTAUROS_WARRIOR,
      MONSTER_GENOCIDER_WARRIOR}},
    {Reader::Kanturu2ndAnd3rd, {MONSTER_PERSONA, MONSTER_TWIN_TALE, MONSTER_DREADFEAR}},
    {Reader::Kanturu3rd, {MONSTER_NIGHTMARE, MONSTER_MAYA_HAND_LEFT, MONSTER_MAYA_HAND_RIGHT, MONSTER_MAYA}},
    {Reader::BalgassBarracks,
     {MONSTER_BALRAM_TRAINEE_SOLDIER, MONSTER_DEATH_SPIRIT_TRAINEE_SOLDIER, MONSTER_SORAM_TRAINEE_SOLDIER,
      MONSTER_DARK_ELF_TRAINEE_SOLDIER}},
    {Reader::IllusionTemple,
     {MONSTER_ILLUSION_SORCERER_SPIRIT1_POISON, MONSTER_ILLUSION_SORCERER_SPIRIT2_POISON,
      MONSTER_ILLUSION_SORCERER_SPIRIT3_POISON, MONSTER_ILLUSION_SORCERER_SPIRIT4_POISON,
      MONSTER_ILLUSION_SORCERER_SPIRIT5_POISON, MONSTER_ILLUSION_SORCERER_SPIRIT6_POISON}},
    {Reader::Raklion,
     {MONSTER_ICE_WALKER, MONSTER_GIANT_MAMMOTH, MONSTER_ICE_GIANT, MONSTER_COOLUTIN, MONSTER_IRON_KNIGHT,
      MONSTER_SELUPAN, MONSTER_DARK_MAMMOTH, MONSTER_DARK_GIANT, MONSTER_DARK_IRON_KNIGHT, MONSTER_DARK_COOLUTIN}},
    {Reader::EmpireGuardian,
     {MONSTER_RAYMOND, MONSTER_LUCAS, MONSTER_FRED, MONSTER_DEVIL_LORD, MONSTER_COMBAT_INSTRUCTOR, MONSTER_DEFENDER,
      MONSTER_FORSAKER}},
    {Reader::EmpireGuardian2, {MONSTER_HAMMERIZE, MONSTER_ATICLES_HEAD, MONSTER_DARK_GHOST}},
    {Reader::EmpireGuardian3, {MONSTER_DUAL_BERSERKER, MONSTER_HEAD_MOUNTER, MONSTER_BANSHEE}},
    {Reader::EmpireGuardian4, {MONSTER_GAYION_THE_GLADIATOR, MONSTER_JERRY}},
}};

constexpr int SwampModels[] = {MODEL_MEDUSA, MODEL_SAPI_QUEEN, MODEL_WOLF_STATUS, MODEL_ICE_NAPIN, MODEL_SHADOW_MASTER};

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
    // SetPlayerAttack answers for the traps and Bali before any handler.
    if (Contains(TrapModels, modelSlot))
    {
        readers.push_back(Reader::Traps);
        return readers;
    }
    if (modelSlot == MODEL_BALI)
    {
        readers.push_back(Reader::Bali);
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
    if (Contains(SwampModels, modelSlot))
    {
        readers.push_back(Reader::SwampOfQuiet);
    }
    // In the order SetPlayerAttack asks them: the first that answers plays.
    std::sort(readers.begin(), readers.end());
    return readers;
}
} // namespace MuEditor::Monsters::AttackReaders

#endif // _EDITOR
