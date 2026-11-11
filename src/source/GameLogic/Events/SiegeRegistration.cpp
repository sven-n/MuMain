#include "stdafx.h"

#include "GameLogic/Events/SiegeRegistration.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Guild/GuildTypes.h"

CSiegeRegistration g_SiegeRegistration;

bool CSiegeRegistration::IsSufficientDeclareLevel() const
{
    if (Hero->GuildStatus != G_MASTER)
        return false;

    return CharacterAttribute->Level >= DeclareLevel;
}
