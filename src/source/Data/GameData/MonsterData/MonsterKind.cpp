#include "stdafx.h"

#include "MonsterKind.h"

#include "Core/Globals/_define.h"

#include <algorithm>
#include <array>
#include <iterator>

namespace Data::Monsters
{
namespace
{
struct KindEntry
{
    MonsterKind kind;
    const char* name;
    int objectKind;
};

constexpr std::array<KindEntry, 4> Kinds = {{
    {MonsterKind::Monster, "monster", KIND_MONSTER},
    {MonsterKind::Npc, "npc", KIND_NPC},
    {MonsterKind::Trap, "trap", KIND_TRAP},
    {MonsterKind::Other, "other", KIND_TMP},
}};

const KindEntry& EntryOf(MonsterKind kind)
{
    return Kinds[static_cast<std::size_t>(kind)];
}
} // namespace

const char* GetMonsterKindName(MonsterKind kind)
{
    return EntryOf(kind).name;
}

std::optional<MonsterKind> FindMonsterKind(std::string_view name)
{
    const auto found =
        std::find_if(Kinds.begin(), Kinds.end(), [&](const KindEntry& entry) { return name == entry.name; });
    if (found == Kinds.end())
    {
        return std::nullopt;
    }
    return found->kind;
}

int ToObjectKind(MonsterKind kind)
{
    return EntryOf(kind).objectKind;
}

std::optional<MonsterKind> FromObjectKind(int objectKind)
{
    const auto found = std::find_if(Kinds.begin(), Kinds.end(),
                                    [&](const KindEntry& entry) { return entry.objectKind == objectKind; });
    if (found == Kinds.end())
    {
        return std::nullopt;
    }
    return found->kind;
}
} // namespace Data::Monsters
