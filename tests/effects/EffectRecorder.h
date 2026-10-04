#pragma once

#include "Data/GameData/EffectData/EffectCreateParams.h"

#include <array>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The effect recorder of the effect catalogue (D40 in
// docs/superpowers/specs/2026-10-02-effect-catalogue-design.md): records what
// one CreateEffect call changes, so that moving an effect's creation from code
// into data can be compared with the old code.
//
// A record holds every field that changed in every slot of the effect pools
// (effects, skill effects, sprites, particles, joints), in the hero and the
// monster owner and in the call's own position, angle and light, plus how many
// values rand() and Random:: gave. The world is pinned (the seeds, the frame
// factor, WorldTime, the scene, the map, the hero), and every pool slot starts
// from pattern A or pattern B, so a case that keeps or reads an old value of a
// slot shows up.
//
// A phase that moves cases records them with the old cases still in the code
// (EffectTestData::BuildShippedRegistry without the new rows) and with the
// rows of the catalogue, in the same build, and compares the records; then it
// deletes the cases.
namespace EffectRecorder
{
enum class Owner
{
    None,
    Hero,
    Monster,
};

enum class SlotPattern
{
    A,
    B,
};

// One CreateEffect call. Position, angle and light default to uneven values,
// so a case that copies one of them into another field shows which one it
// copied; the other arguments default to the game's default arguments.
struct EffectCall
{
    int type = 0;
    int subType = 0;
    Owner owner = Owner::None;
    std::array<float, 3> position{13120.25f, 12480.5f, 140.75f};
    std::array<float, 3> angle{11.f, 22.f, 33.f};
    std::array<float, 3> light{0.9f, 0.8f, 0.7f};
    short pkKey = -1;
    std::uint16_t skillIndex = 0;
    std::uint16_t skill = 0;
    std::uint16_t skillSerialNum = 0;
    float scale = 0.f;
    short targetIndex = -1;
};

struct Conditions
{
    float frameFactor = 1.f;
    SlotPattern pattern = SlotPattern::A;
};

// The frame factors 1.0 and 0.5, each with pattern A and B.
std::span<const Conditions> AllConditions();

// Each sub type with each owner (none, the hero, a monster), once with the
// game's default arguments and once with uneven values for the scale, the PK
// key, the skill values and the target index (so a case that keeps the
// caller's scale differs from one that sets the default scale).
std::vector<EffectCall> CallsFor(int type, std::initializer_list<int> subTypes);

// The calls of CallsFor, plus, for each sub type without an owner, both
// argument sets with a second position, angle and light. Every component
// differs from the first set and from the values the cases set, so a row that
// copies one of them differs from a row that sets the copied value as a
// constant, and a row that sets single components from one that sets the
// whole vector.
std::vector<EffectCall> SecondGeometryCallsFor(int type, std::initializer_list<int> subTypes);

struct RecordedValue
{
    std::string path;
    std::string value;

    bool operator==(const RecordedValue&) const = default;
};

// The values that differ from before the call, in pool, slot and field order,
// after the draw counts ("draws.rand", "draws.Random").
using Record = std::vector<RecordedValue>;

struct Difference
{
    std::string path;
    std::string expected; // "unchanged" when only the other record has the path
    std::string actual;
};

Record RecordCall(const EffectCall& call, const Conditions& conditions);
std::vector<Difference> Compare(const Record& expected, const Record& actual);
std::optional<std::string> Find(const Record& record, std::string_view path);

// A hash of the whole record, the same on every platform for the same record.
std::uint64_t Digest(const Record& record);

std::string Describe(const EffectCall& call, const Conditions& conditions);
std::string ToText(const Record& record);
std::string ToText(const std::vector<Difference>& differences);

} // namespace EffectRecorder
