#include "stdafx.h"

#ifdef _EDITOR

#include "SoundKeyFollow.h"
#include "SoundEditorRows.h"

#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"

#include <algorithm>
#include <optional>
#include <string_view>
#include <unordered_map>

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

// The sets of a table by their names, which a lookup finds without going
// through all of them.
class SetIndex
{
public:
    explicit SetIndex(const std::vector<MonsterSoundSet>& sets)
    {
        m_byName.reserve(sets.size());
        for (const MonsterSoundSet& set : sets)
        {
            m_byName.emplace(set.name, &set);
        }
    }

    const MonsterSoundSet* Find(std::string_view name) const
    {
        const auto found = m_byName.find(name);
        return found != m_byName.end() ? found->second : nullptr;
    }

private:
    std::unordered_map<std::string_view, const MonsterSoundSet*> m_byName;
};

// Two tables, the sets of each by name, and the sets known to be renamed
// from one to the other (before's name to after's).
struct SetLookup
{
    SetLookup(const KeyedTables& beforeTables, const KeyedTables& afterTables,
              const std::map<std::string, std::string>& knownNames)
        : before(beforeTables), after(afterTables), beforeByName(beforeTables.sets), afterByName(afterTables.sets),
          names(knownNames)
    {
    }

    const KeyedTables& before;
    const KeyedTables& after;
    SetIndex beforeByName;
    SetIndex afterByName;
    const std::map<std::string, std::string>& names;
};

bool AlikeButName(const MonsterSoundSet& left, MonsterSoundSet right)
{
    right.name = left.name;
    return left == right;
}

bool IsKnownName(const std::map<std::string, std::string>& names, const std::string& name)
{
    return std::any_of(names.begin(), names.end(), [&name](const auto& entry) { return entry.second == name; });
}

// A set of `after` that `before` does not have by name and no known rename
// (`names`) takes, alike `set` but its name, when it is the only one and no
// other set gone from `before` is alike.
const MonsterSoundSet* RenamedTo(const MonsterSoundSet& set, const SetLookup& lookup)
{
    const MonsterSoundSet* renamed = nullptr;
    for (const MonsterSoundSet& candidate : lookup.after.sets)
    {
        if (lookup.beforeByName.Find(candidate.name) != nullptr || IsKnownName(lookup.names, candidate.name) ||
            !AlikeButName(set, candidate))
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
        lookup.before.sets.begin(), lookup.before.sets.end(), [&](const MonsterSoundSet& other)
        { return &other != &set && lookup.afterByName.Find(other.name) == nullptr && AlikeButName(other, *renamed); });
    return another ? nullptr : renamed;
}

// The set of `after` that `set` of `before` is: by a name known for it, by
// its own, else renamed.
const MonsterSoundSet* SetAfter(const MonsterSoundSet& set, const SetLookup& lookup)
{
    if (const auto known = lookup.names.find(set.name); known != lookup.names.end())
    {
        return lookup.afterByName.Find(known->second);
    }
    const MonsterSoundSet* now = lookup.afterByName.Find(set.name);
    return now != nullptr ? now : RenamedTo(set, lookup);
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

std::vector<MonsterSoundSet> CurrentKeyedSets()
{
    std::vector<MonsterSoundSet> sets;
    sets.reserve(g_MonsterSoundSetDatabase.GetAll().size());
    for (const MonsterSoundSet* set : g_MonsterSoundSetDatabase.GetAll())
    {
        sets.push_back(*set);
    }
    return sets;
}

std::map<std::string, std::string> FollowedKeys(const KeyedTables& before, const KeyedTables& after,
                                                const std::map<std::string, std::string>& names)
{
    const PlacedKeys from = KeysOf(before);
    const PlacedKeys to = KeysOf(after);
    const SetLookup lookup(before, after, names);
    std::map<std::string, std::string> renamed;
    for (const MonsterSoundSet& set : before.sets)
    {
        if (const MonsterSoundSet* now = SetAfter(set, lookup))
        {
            Follow(from.sets.at(set.name), to.sets.at(now->name), PlacesAfter(set.sounds, now->sounds), renamed);
        }
    }
    Follow(from.objects, to.objects, PlacesAfter(before.objects, after.objects), renamed);
    return renamed;
}

std::map<std::string, std::string> RenamedSets(const KeyedTables& before, const KeyedTables& after)
{
    const std::map<std::string, std::string> none;
    const SetLookup lookup(before, after, none);
    std::map<std::string, std::string> renamed;
    for (const MonsterSoundSet& set : before.sets)
    {
        const MonsterSoundSet* now = SetAfter(set, lookup);
        if (now != nullptr && now->name != set.name)
        {
            renamed.emplace(set.name, now->name);
        }
    }
    return renamed;
}
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
