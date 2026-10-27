#include "stdafx.h"
#include "Engine/Object/Movement/LookAt.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/Movement/FrameTime.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Terrain/ZzzLodTerrain.h"

#include <algorithm>
#include <cstdlib>

namespace Engine::Characters
{
namespace
{
// What a head looks at: enemies this many tiles away at most, NPCs and other players (and an NPC the hero) this many,
// a monster its target this many, within LookViewDegrees of where the character faces, with no wall between. A player
// looks around for another this often, and otherwise keeps the one it looks at while that stays in sight.
constexpr int LookEnemyTiles = 6;
constexpr int LookNearbyTiles = 4;
constexpr int LookMonsterTargetTiles = 8;
constexpr float LookScanSeconds = 0.25f;

bool InSight(const CHARACTER* c, const CHARACTER* target, int tiles)
{
    if (target == c || !target->Object.Live || target->Dead != 0)
    {
        return false;
    }
    const int dx = target->PositionX - c->PositionX;
    const int dy = target->PositionY - c->PositionY;
    const int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps > tiles)
    {
        return false;
    }
    const float toTarget = CreateAngle2D(c->Object.Position, target->Object.Position);
    if (FarAngle(c->Object.Angle[2], toTarget) >= LookViewDegrees)
    {
        return false;
    }
    // A wall (a tile no one walks through) on the line between them blocks the view.
    for (int step = 1; step < steps; ++step)
    {
        const int x = c->PositionX + (dx * step + (dx > 0 ? steps / 2 : -steps / 2)) / steps;
        const int y = c->PositionY + (dy * step + (dy > 0 ? steps / 2 : -steps / 2)) / steps;
        if ((TerrainWall[TERRAIN_INDEX(x, y)] & TW_NOMOVE) != 0)
        {
            return false;
        }
    }
    return true;
}

const CHARACTER* CharacterAt(int index)
{
    return index >= 0 && index < MAX_CHARACTERS_CLIENT ? &CharactersClient[index] : nullptr;
}

int LookTiles(const CHARACTER* target)
{
    return target->Object.Kind == KIND_MONSTER ? LookEnemyTiles : LookNearbyTiles;
}

// The nearest character of `kind` in sight, or -1.
int NearestInSight(const CHARACTER* c, int kind, int tiles)
{
    int nearest = -1;
    int nearestSteps = tiles + 1;
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        const CHARACTER* other = &CharactersClient[i];
        if (other->Object.Kind != kind || !other->Object.Live)
        {
            continue;
        }
        const int steps =
            std::max(std::abs(other->PositionX - c->PositionX), std::abs(other->PositionY - c->PositionY));
        if (steps < nearestSteps && InSight(c, other, tiles))
        {
            nearest = i;
            nearestSteps = steps;
        }
    }
    return nearest;
}

// A player's pick: what it attacks, then the one it looks at while that's still the kind it would pick, then the
// nearest of that kind. In a combat zone enemies come first; then NPCs, then other players passing by.
int PickPlayerLook(const CHARACTER* c)
{
    const bool combat = !c->SafeZone;
    if (combat)
    {
        const CHARACTER* attacked = CharacterAt(c->TargetCharacter);
        if (attacked != nullptr && attacked->Object.Kind == KIND_MONSTER && InSight(c, attacked, LookEnemyTiles))
        {
            return c->TargetCharacter;
        }
    }
    const CHARACTER* current = CharacterAt(c->Motion.LookTarget);
    const bool keepCurrent = current != nullptr && InSight(c, current, LookTiles(current));
    if (combat)
    {
        if (keepCurrent && current->Object.Kind == KIND_MONSTER)
        {
            return c->Motion.LookTarget;
        }
        const int enemy = NearestInSight(c, KIND_MONSTER, LookEnemyTiles);
        if (enemy >= 0)
        {
            return enemy;
        }
    }
    if (keepCurrent && current->Object.Kind == KIND_NPC)
    {
        return c->Motion.LookTarget;
    }
    const int npc = NearestInSight(c, KIND_NPC, LookNearbyTiles);
    if (npc >= 0)
    {
        return npc;
    }
    if (keepCurrent && current->Object.Kind == KIND_PLAYER)
    {
        return c->Motion.LookTarget;
    }
    return NearestInSight(c, KIND_PLAYER, LookNearbyTiles);
}
} // namespace

const CHARACTER* UpdateLookAt(CHARACTER* c)
{
    const OBJECT* o = &c->Object;
    if (o->Kind == KIND_MONSTER)
    {
        // A monster looks at its target, while it's in sight.
        const CHARACTER* target = CharacterAt(c->TargetCharacter);
        c->Motion.LookTarget =
            target != nullptr && InSight(c, target, LookMonsterTargetTiles) ? c->TargetCharacter : -1;
        return CharacterAt(c->Motion.LookTarget);
    }
    if (o->Kind == KIND_NPC)
    {
        // An NPC looks at the hero passing by.
        c->Motion.LookTarget =
            Hero != nullptr && InSight(c, Hero, LookNearbyTiles) ? static_cast<short>(Hero - CharactersClient) : -1;
        return CharacterAt(c->Motion.LookTarget);
    }
    if (o->Kind != KIND_PLAYER)
    {
        c->Motion.LookTarget = -1;
        return nullptr;
    }
    // A player looks around only now and then (each on its own beat): in between it keeps the one it looks at, while
    // that stays in sight.
    c->Motion.LookScanSeconds -= FrameSeconds();
    if (c->Motion.LookScanSeconds <= 0.f)
    {
        c->Motion.LookScanSeconds = LookScanSeconds + static_cast<float>(c->Key % 8) * (LookScanSeconds / 8.f);
        c->Motion.LookTarget = static_cast<short>(PickPlayerLook(c));
    }
    else if (const CHARACTER* current = CharacterAt(c->Motion.LookTarget);
             current != nullptr && !InSight(c, current, LookTiles(current)))
    {
        c->Motion.LookTarget = -1;
    }
    return CharacterAt(c->Motion.LookTarget);
}

void LookTowards(OBJECT* o, const CHARACTER* target, float maxDegrees)
{
    const OBJECT* t = &target->Object;
    const float yaw =
        std::clamp(FarAngle(CreateAngle2D(o->Position, t->Position), o->Angle[2], false), -maxDegrees, maxDegrees);
    o->HeadTargetAngle[0] = yaw < 0.f ? yaw + 360.f : yaw;
    // Up or down toward its middle, from about the eyes, as LookAtTarget.
    const float pitch = (t->Position[2] - (o->Position[2] + LookEyeHeight)) * LookPitchPerUnit;
    o->HeadTargetAngle[1] = pitch < 0.f ? pitch + 360.f : pitch;
}
} // namespace Engine::Characters
