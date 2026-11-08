#include "stdafx.h"

#ifdef _EDITOR

#include "Audio/SoundLoads.h"

#include <map>

namespace Audio::SoundLoads
{
namespace
{
struct State
{
    std::optional<int> where;
    std::map<int, int> loaded;
    std::set<int> playedUnloaded;
};

State& Loads()
{
    static State state;
    return state;
}
} // namespace

ScopedLoadWhere::ScopedLoadWhere(int world) : m_outer(Loads().where)
{
    Loads().where = world;
}

ScopedLoadWhere::~ScopedLoadWhere()
{
    Loads().where = m_outer;
}

void NoteLoad(int number)
{
    State& state = Loads();
    state.loaded.try_emplace(number, state.where.value_or(AtStart));
    state.playedUnloaded.erase(number);
}

std::optional<int> WhereLoaded(int number)
{
    const auto found = Loads().loaded.find(number);
    if (found == Loads().loaded.end())
    {
        return std::nullopt;
    }
    return found->second;
}

void NotePlayedUnloaded(int number)
{
    Loads().playedUnloaded.insert(number);
}

const std::set<int>& PlayedUnloaded()
{
    return Loads().playedUnloaded;
}

void Clear()
{
    Loads() = State{};
}
} // namespace Audio::SoundLoads

#endif // _EDITOR
