#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace Data::Monsters
{
// What the game does with a character of a monster number: attack it, talk
// to it, ignore it as a trap. It becomes the character's OBJECT::Kind.
enum class MonsterKind : std::uint8_t
{
    Monster,
    Npc,
    Trap,
    // KIND_TMP: neither attacked nor talked to (451).
    Other,
};

// The kind's name in the data ("monster", "npc", "trap", "other"), and back.
const char* GetMonsterKindName(MonsterKind kind);
std::optional<MonsterKind> FindMonsterKind(std::string_view name);

// The OBJECT::Kind of a character of this kind (KIND_MONSTER, KIND_NPC,
// KIND_TRAP, KIND_TMP), and back; nothing for an object kind no monster has.
int ToObjectKind(MonsterKind kind);
std::optional<MonsterKind> FromObjectKind(int objectKind);
} // namespace Data::Monsters
