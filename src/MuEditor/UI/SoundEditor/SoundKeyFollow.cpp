#include "stdafx.h"

#ifdef _EDITOR

#include "SoundKeyFollow.h"
#include "SoundEditorRows.h"

#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"

#include <algorithm>
#include <optional>

namespace MuEditor::SoundEditor
{
namespace
{
using Data::Monsters::MonsterSoundSet;

// The keys of the tables' sounds by their place: a set's by its name.
struct PlacedKeys
{
    std::map<std::string, std::vector<std::string>> sets;
    std::vector<std::string> objects;
};

PlacedKeys KeysOf(const KeyedTables& tables)
{
    PlacedKeys keys;
    for (const MonsterSoundSet& set : tables.sets)
    {
        keys.sets[set.name];
    }
    // KeyRows lists each set's sounds in their order.
    for (const SoundRow& row : KeyRows(tables.sets, tables.objects))
    {
        if (row.source == RowSource::MonsterSet)
        {
            keys.sets[row.set].push_back(row.key);
        }
        else
        {
            keys.objects.push_back(row.key);
        }
    }
    return keys;
}

// Where each entry of `before` is in `after`, or none (it went): the entries
// alike at the start and at the end keep theirs, and the ones between changed
// in their places when as many are left on both sides.
template <typename Entry>
std::vector<std::optional<size_t>> PlacesAfter(const std::vector<Entry>& before, const std::vector<Entry>& after)
{
    std::vector<std::optional<size_t>> places(before.size());
    size_t head = 0;
    while (head < before.size() && head < after.size() && before[head] == after[head])
    {
        places[head] = head;
        ++head;
    }
    size_t tail = 0;
    while (head + tail < before.size() && head + tail < after.size() &&
           before[before.size() - 1 - tail] == after[after.size() - 1 - tail])
    {
        places[before.size() - 1 - tail] = after.size() - 1 - tail;
        ++tail;
    }
    if (before.size() == after.size())
    {
        for (size_t index = head; index + tail < before.size(); ++index)
        {
            places[index] = index;
        }
    }
    return places;
}

const MonsterSoundSet* Named(const std::vector<MonsterSoundSet>& sets, const std::string& name)
{
    const auto found =
        std::find_if(sets.begin(), sets.end(), [&name](const MonsterSoundSet& set) { return set.name == name; });
    return found != sets.end() ? &*found : nullptr;
}

bool AlikeButName(const MonsterSoundSet& left, MonsterSoundSet right)
{
    right.name = left.name;
    return left == right;
}

// A set of `after` that `before` does not have by name, alike `set` but its
// name, when it is the only one and no other set gone from `before` is alike.
const MonsterSoundSet* RenamedTo(const MonsterSoundSet& set, const KeyedTables& before, const KeyedTables& after)
{
    const MonsterSoundSet* renamed = nullptr;
    for (const MonsterSoundSet& candidate : after.sets)
    {
        if (Named(before.sets, candidate.name) != nullptr || !AlikeButName(set, candidate))
        {
            continue;
        }
        if (renamed != nullptr)
        {
            return nullptr;
        }
        renamed = &candidate;
    }
    if (renamed == nullptr)
    {
        return nullptr;
    }
    const bool another = std::any_of(
        before.sets.begin(), before.sets.end(), [&](const MonsterSoundSet& other)
        { return &other != &set && Named(after.sets, other.name) == nullptr && AlikeButName(other, *renamed); });
    return another ? nullptr : renamed;
}

// The set of `after` that `set` of `before` is: by its name, else renamed.
const MonsterSoundSet* SetAfter(const MonsterSoundSet& set, const KeyedTables& before, const KeyedTables& after)
{
    const MonsterSoundSet* now = Named(after.sets, set.name);
    return now != nullptr ? now : RenamedTo(set, before, after);
}

void Follow(const std::vector<std::string>& from, const std::vector<std::string>& to,
            const std::vector<std::optional<size_t>>& places, std::map<std::string, std::string>& renamed)
{
    for (size_t index = 0; index < places.size(); ++index)
    {
        if (places[index] && from[index] != to[*places[index]])
        {
            renamed.emplace(from[index], to[*places[index]]);
        }
    }
}
} // namespace

KeyedTables CurrentKeyedTables()
{
    KeyedTables tables;
    for (const MonsterSoundSet* set : g_MonsterSoundSetDatabase.GetAll())
    {
        tables.sets.push_back(*set);
    }
    tables.objects = g_MapObjectSoundDatabase.GetAll();
    return tables;
}

std::map<std::string, std::string> FollowedKeys(const KeyedTables& before, const KeyedTables& after)
{
    const PlacedKeys from = KeysOf(before);
    const PlacedKeys to = KeysOf(after);
    std::map<std::string, std::string> renamed;
    for (const MonsterSoundSet& set : before.sets)
    {
        if (const MonsterSoundSet* now = SetAfter(set, before, after))
        {
            Follow(from.sets.at(set.name), to.sets.at(now->name), PlacesAfter(set.sounds, now->sounds), renamed);
        }
    }
    Follow(from.objects, to.objects, PlacesAfter(before.objects, after.objects), renamed);
    return renamed;
}

std::map<std::string, std::string> RenamedSets(const KeyedTables& before, const KeyedTables& after)
{
    std::map<std::string, std::string> renamed;
    for (const MonsterSoundSet& set : before.sets)
    {
        const MonsterSoundSet* now = SetAfter(set, before, after);
        if (now != nullptr && now->name != set.name)
        {
            renamed.emplace(set.name, now->name);
        }
    }
    return renamed;
}
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
