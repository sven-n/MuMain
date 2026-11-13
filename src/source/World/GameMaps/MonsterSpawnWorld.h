#pragma once

#include "GameLogic/Monsters/MonsterSetupApply.h"

#include <string_view>

class CHARACTER;

namespace Data::Monsters
{
struct MonsterHook;
} // namespace Data::Monsters

// What the maps tell the monster data's setup when a monster appears
// (monsters design, MN4b): the states its state variants name, and the hooks
// it hands over to.
namespace World::Monsters
{
// Whether a state of Data::Monsters::GetMonsterStates is on now (Q-MN36c):
// the castle siege running or not, Raklion's boss waiting to drop, a player
// transformed into the monster (PlayerTransformScope), Crywolf's battle
// about to start.
bool IsSpawnStateOn(std::string_view state);

// While it lives, the monsters made are for players transformed into them
// (0x45): the state playerTransformed is on (Q-MN38d).
class PlayerTransformScope
{
public:
    PlayerTransformScope();
    ~PlayerTransformScope();

    PlayerTransformScope(const PlayerTransformScope&) = delete;
    PlayerTransformScope& operator=(const PlayerTransformScope&) = delete;

private:
    bool m_previous;
};

// Runs a hook of Data::Monsters::GetMonsterHookKinds for the character that
// appeared (Q-MN36d); one the maps do not know does nothing.
void RunSpawnHook(const Data::Monsters::MonsterHook& hook, CHARACTER& character);

// For Setup::Create.
const GameLogic::Monsters::Setup::SpawnWorld& GetSpawnWorld();

// Setup::ApplyLive with GetSpawnWorld: the living monsters of the number take
// what changed at once (D55). MonsterDataHandler tells it the changes.
void ApplyLiveSetup(int number);

// Makes the living monsters of the number again, for a test of the next
// spawn's fields (D55, the monster tools' Respawn live monsters): the same
// key, slot, tile and turn; what the last spawn attached goes first; with
// `playAppear` they come in as the server's new monsters do. The server is
// not told. Returns how many.
int RespawnLive(int number, bool playAppear);
} // namespace World::Monsters
