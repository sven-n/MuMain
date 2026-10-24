#include "stdafx.h"

#include "GameLogic/Monsters/MonsterNames.h"

#include "Core/Text/WideString.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Engine/Object/ZzzCharacter.h"

#include <iterator>

namespace GameLogic::Monsters::Names
{
void WriteShownName(CHARACTER& character, int number)
{
    if (const wchar_t* name = g_MonsterDatabase.FindDisplayName(number))
    {
        Core::Text::CopyWideString(character.ID, std::size(character.ID), name);
    }
}

void RefreshShownNames(int number)
{
    if (CharactersClient == nullptr)
    {
        return;
    }
    for (int index = 0; index < MAX_CHARACTERS_CLIENT; ++index)
    {
        CHARACTER& character = CharactersClient[index];
        if (!character.Object.Live || character.MonsterIndex == MONSTER_UNDEFINED)
        {
            continue;
        }
        // Players, and the monsters that carry a player's name (summons,
        // transformed players, Kalima gates), have the player kind.
        if (character.Object.Kind == KIND_PLAYER || character.Change)
        {
            continue;
        }
        if (number != Data::Monsters::MonsterDataHandler::AllNumbers && character.MonsterIndex != number)
        {
            continue;
        }
        WriteShownName(character, character.MonsterIndex);
    }
}
} // namespace GameLogic::Monsters::Names
