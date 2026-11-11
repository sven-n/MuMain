#pragma once

#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

class CHARACTER;
class OBJECT;

namespace Data::Maps
{
struct MapRef;
} // namespace Data::Maps

namespace Data::Monsters
{
struct MonsterDefinition;
struct MonsterSetup;
struct MonsterGate;
struct MonsterAppear;
struct MonsterHook;
enum class UnknownReason;
} // namespace Data::Monsters

// A monster's character made from its entry in the monster list (monsters
// design, D63, MN4): the setup the creators in the code wrote, as data.
namespace GameLogic::Monsters::Setup
{
// What the maps give the setup when a monster appears: whether a state of
// Data::Monsters::GetMonsterStates is on (Q-MN36c), and the hooks they run
// (Q-MN36d), after the setup. CreateMonster gives
// World::Monsters::GetSpawnWorld.
struct SpawnWorld
{
    std::function<bool(std::string_view state)> isStateOn;
    std::function<void(const Data::Monsters::MonsterHook& hook, CHARACTER& character)> runHook;
    // True while the monster tools' Respawn makes living monsters again: the
    // hooks and the objects a row puts beside the monster ran at their spawn
    // and stay, so Create runs and makes them no more.
    std::function<bool()> isRespawn;
};

// How a monster the server has just spawned comes in (0x13 and 0x1F with
// their new flag), and one that teleported (0x13's flag), from its entry's
// appear and teleport on the map and in the states that are on (Q-MN38c). A
// number without an entry comes in the model's way.
void Appear(CHARACTER& character, const SpawnWorld& world);
void Teleport(CHARACTER& character, const SpawnWorld& world);

// The animation an appear (or a teleport) plays on the model: its own, else,
// when it fills what it leaves out by the model (`byModel`, an appear), a
// player-shaped model's PLAYER_COME_UP; nullopt for none.
std::optional<int> AppearAction(const Data::Monsters::MonsterAppear& appear, int modelSlot, bool byModel);

// The castle gate of a character: its entry's gate on the map and in the
// states that are on; nullptr for one without (Q-MN38c). It points into the
// monster list.
const Data::Monsters::MonsterGate* GateOf(const CHARACTER& character, const SpawnWorld& world);

// Makes the spawn effects at bones the character's entry gave it (a joint,
// effect or particle with a bone), once, at its first frame on screen: the
// bones have a pose only then. MoveCharacterVisual calls it.
void RunWaitingSpawnEffects(CHARACTER& character);

// Opens the model of the slot from the model table, a monster's or an NPC's;
// nothing for a slot the table has no entry for.
void OpenModel(int slot);

// Makes the character of the entry at the tile with the key, as the code's
// creators did: draws a random look, opens the model, creates the character
// and gives it the setup of the map and of the states that are on. CreateMonster
// makes every number with an entry so, on every map (Q-MN35). The body stands
// at the setup's tile offset; the character's tile stays the given one, the
// server's. nullptr when the
// entry names no model slot (NoModel), or when the model's file is missing and
// the unknown monsters' look is for that (MissingFile, D64); why, when given,
// says which. Another missing file makes a character without a body.
CHARACTER* Create(const Data::Monsters::MonsterDefinition& entry, int positionX, int positionY, int key,
                  const SpawnWorld& world, Data::Monsters::UnknownReason* why = nullptr);

// The setup a field comes from, as SetupNow takes it, without making one: a
// state variant that is on and sets it, else the map variant on the server's
// map that sets it, else the entry's own. `isSet` says whether a setup sets
// the field. The reference points into the monster list.
const Data::Monsters::MonsterSetup& SetupWithField(const Data::Monsters::MonsterDefinition& entry,
                                                   bool (*isSet)(const Data::Monsters::MonsterSetup& setup),
                                                   const SpawnWorld& world);

// The setup of the entry on the server's map with the fields of the state
// variants that are on now, as a spawn takes it.
Data::Monsters::MonsterSetup SetupNow(const Data::Monsters::MonsterDefinition& entry, const SpawnWorld& world);

// The setup of the entry on the map (nullopt: on all maps, the entry's own)
// with the fields of its state variant of `state` (empty: none), as Monster
// Preview's Map and State show it.
Data::Monsters::MonsterSetup SetupFor(const Data::Monsters::MonsterDefinition& entry,
                                      const std::optional<Data::Maps::MapRef>& map, std::string_view state);

// Dresses the character as the setup has it: its equipment, player class and
// class step, and its look (scale, skin, render tier, glow and hidden mesh,
// outline, shadow, alpha, light, PK colour, plate lights). Create does it at
// spawn before the rest of the setup; Monster Preview's character too.
void Dress(CHARACTER& character, const Data::Monsters::MonsterSetup& setup);

// Gives the living characters of the number (a negative number: of every
// number) the fields living monsters take at once (D55, Q-MN39: their look,
// equipment, speed and turning, fixed facing) as their entry has them now, on
// the map and in the states that are on, with the random look each drew: from
// the values each had before its setup, so a field the entry no longer has
// goes back to the game's. The other fields wait for the next spawn. The
// editor's monster changes reach the game so. A character a player stands
// behind (a summon, a transformed player) and one with the unknown monsters'
// look are left alone.
void ApplyLive(int number, const SpawnWorld& world);
// The slot gets a new character (CreateCharacter), or is freed: its start
// and its waiting spawn effects are dropped; Create notes it again.
void ForgetLive(const CHARACTER& character);

// The living characters of the number that ApplyLive changes (the monster
// tools' Respawn makes them again).
std::vector<CHARACTER*> FindLive(int number);
// How many spawns were noted so far: it changes with each spawn, a respawn
// into the same character too.
int GetLiveSpawnCount();
// How many of them appeared with another value of each setup field than
// their entry gives now, added to `waiting` by the field's key (and "model"):
// the change waits for their next spawn (D55). Each character's setup is made
// once; Monster Setup counts a frame so.
using WaitingCounts = std::map<std::string, int, std::less<>>;
void CountWaiting(int number, const SpawnWorld& world, WaitingCounts& waiting);
// Removes what the character's last spawn attached to it: the joints and
// effects of its spawn effects, its part models, pet and cloth. Respawn calls
// it before it makes the character again.
void RemoveSpawnAttachments(CHARACTER& character);

// A living character got a buff by WSclient's RegisterBuff (the buff lists
// of 0x13, 0x1F and 0x45, 0x07's add, also for a buff it shows already from
// its spawn's setup) or lost one by its UnRegisterBuff (0x07's clear, the
// skill ends of 0x1B, and a few client paths such as stun and sleep ending).
// Respawn keeps a setup buff noted so. Characters that are no living
// monsters are left alone.
void NoteServerBuff(const OBJECT& object, int buff, bool given);

// What the server gave a living character, which CreateCharacter clears:
// the buffs it shows except its last spawn's setup buffs that the server did
// not give too (NoteServerBuff), and its build time and health and shield
// bars.
// Respawn keeps them across making it again.
struct ServerState
{
    std::vector<int> buffs;
    // Those of `buffs` NoteServerBuff noted, noted again on the new
    // character; a note of a buff it no longer shows is not carried over.
    std::vector<int> serverBuffs;
    unsigned char buildTime = 0;
    float health = -1.0f;
    float shield = -1.0f;
};
ServerState KeepServerState(const CHARACTER& character);
void RestoreServerState(CHARACTER& character, const ServerState& state);

// Monster Preview's spawn and appear (MN4): the setup's spawn effects on a
// character that has a pose, as a spawn makes them; its appear as a spawn
// fills it by the model, with its sound only when `sound`. Neither makes the
// objects a row puts beside the monster: they would go into the map.
void ReplaySpawnEffects(CHARACTER& character, const Data::Monsters::MonsterSetup& setup);
void ReplayAppear(CHARACTER& character, const Data::Monsters::MonsterSetup& setup, bool sound);
// Ends the joints, effects, particles and sprites that are on the object or
// follow it, as when it goes away (Monster Preview's character).
void RemoveEffectsOf(const OBJECT& object);
} // namespace GameLogic::Monsters::Setup
