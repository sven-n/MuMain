#include "stdafx.h"

#include "MonsterFacing.h"

#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Monsters/MonsterSetupApply.h"

namespace GameLogic::Monsters::Facing
{
namespace
{
constexpr float DefaultStep = 45.0f;
} // namespace

float AngleOnArrival(const CHARACTER& character, int direction, const Setup::SpawnWorld& world)
{
    return AngleOnArrival(character.MonsterIndex, character.PositionX, character.PositionY, direction, world);
}

float AngleOnArrival(int number, int tileX, int tileY, int direction, const Setup::SpawnWorld& world)
{
    float step = DefaultStep;
    if (const Data::Monsters::MonsterDefinition* entry = g_MonsterDatabase.Find(number))
    {
        // Each facing field where it comes from, without a whole setup: 0x13
        // asks for every monster it brings.
        using Data::Monsters::MonsterSetup;
        const MonsterSetup& facing =
            Setup::SetupWithField(*entry, [](const MonsterSetup& setup) { return setup.facing.has_value(); }, world);
        if (facing.facing)
            return *facing.facing;
        const MonsterSetup& stepped = Setup::SetupWithField(
            *entry, [](const MonsterSetup& setup) { return setup.facingStep.has_value(); }, world);
        step = stepped.facingStep.value_or(DefaultStep);
        const MonsterSetup& byTile =
            Setup::SetupWithField(*entry, [](const MonsterSetup& setup) { return !setup.facingByTile.empty(); }, world);
        // The last of its tiles it stands on wins, as the Ballista's checks did.
        for (const Data::Monsters::MonsterTileFacing& tile : byTile.facingByTile)
        {
            if ((tile.tileX && *tile.tileX == tileX) || (tile.tileY && *tile.tileY == tileY))
                step = tile.step;
        }
    }
    return (static_cast<float>(direction) - 1.0f) * step;
}
} // namespace GameLogic::Monsters::Facing
