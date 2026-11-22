#pragma once

#include "../data/LookRecorder.h"

#include "Data/GameData/MapData/MapRef.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

class CHARACTER;

// The draw recorder of monster looks (MN8, D50 in docs/superpowers/specs/2026-11-13-monster-looks-design.md): draws
// a monster the creation scope made with the game's own character drawing (RenderCharacter), through the item looks'
// recording drawer (tests/data/LookRecorder.h), at a pinned clock, random key and pose. A look moved from code into
// data keeps the digests its old code drew (the baseline), on each map its numbers are placed on.
namespace MonsterRecorder
{
// How a monster is drawn.
struct DrawSituation
{
    // MONSTER01_ action, and its frame.
    int action = 0;
    float frame = 0.f;
    // Milliseconds of WorldTime.
    double worldTime = 0.0;
    float alpha = 1.f;
    // An eBuffState on the monster, or none.
    std::optional<int> buff;

    // "stop1 at frame 0, 1234567.25 ms, poison", for messages and the baseline.
    [[nodiscard]] std::string Describe() const;
};

// The situations a monster of the model in `slot` is drawn in: standing, walking, attacking and dying (the actions
// its model has), at another clock time, poisoned, and fading; a model whose drawing changes as it fades while it dies
// (the Blade Hunter's) also dying and fading.
std::vector<DrawSituation> DrawSituations(int slot);

// Draws the character as the game draws it and records the drawing; the character's object and model are as they
// were afterwards.
LookRecorder::Record RecordMonster(CHARACTER& character, const DrawSituation& situation);

// The shipped looks of Data/Effects in g_LookStore while it lives, with their textures found; as the game had them
// before afterwards.
class ShippedLooks
{
public:
    ShippedLooks();
    ~ShippedLooks();
    ShippedLooks(const ShippedLooks&) = delete;
    ShippedLooks& operator=(const ShippedLooks&) = delete;

private:
    struct State;
    State* m_state;
};

// The baseline (tests/monsters/baseline/MonsterLookDraws.txt): "<number> on <map> <situation>" and the digest of the
// drawing (LookRecorder::Digest).
std::map<std::string, std::string> ReadBaseline();
void WriteBaseline(const std::map<std::string, std::string>& lines);

// The baseline key of a number drawn on a map in a situation.
std::string BaselineKey(int number, const Data::Maps::MapRef& map, const DrawSituation& situation);
} // namespace MonsterRecorder
