#pragma once

#include <functional>
#include <string>

// The content pack of a monster's files (Data/Maps/Files/Monsters.json): a target that delivers content after start-up
// may not have them when the server spawns the monster, because a map's own pack holds only the monsters the server
// spawns there at start (invasions, events, summons, GM commands send the rest). CreateMonster asks for the pack;
// until it is ready the monster stands in as a Bull Fighter's body, and when it is ready (or failed) the pump makes the
// monster again at the tile it stands on, which replaces the stand-in in place (CreateCharacter with the same key).
namespace GameLogic::Monsters::Packs
{
// A monster that waits for its pack.
struct Waiting
{
    int number = 0;
    int key = 0;
};

// The pack of a monster number (Content::RequestPack), "monster<number>".
std::string PackOf(int number);

// Asks for the monster's pack. True when the monster can be made now: the pack is ready (always, where every file is on
// disk), or failed, which is logged once per monster and leaves the unknown monsters' look to the missing file. False
// when the pack is pending: the monster is remembered and the caller makes the stand-in.
bool ReadyToCreate(int number, int key);

// Once a frame: asks again for each waiting monster; `remake` is called for each whose pack is ready or failed, which
// forgets it.
void Pump(const std::function<void(const Waiting&)>& remake);

// The waiting monsters are dropped: the session is torn down.
void Cancel();

// How many wait.
int WaitingCount();
} // namespace GameLogic::Monsters::Packs
