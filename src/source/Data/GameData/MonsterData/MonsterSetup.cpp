#include "stdafx.h"

#include "MonsterSetup.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"

#include <algorithm>
#include <charconv>
#include <string>
#include <utility>

namespace Data::Monsters
{
namespace
{
// A field held in one member: set when it has a value.
#define MONSTER_SETUP_OPTIONAL(name)                                                                                   \
    MonsterSetupField                                                                                                  \
    {                                                                                                                  \
        #name, [](const MonsterSetup& setup) { return setup.name.has_value(); },                                       \
            [](const MonsterSetup& left, const MonsterSetup& right) { return left.name == right.name; },               \
            [](MonsterSetup& to, const MonsterSetup& from) { to.name = from.name; }                                    \
    }

// A list: set when it is not empty.
#define MONSTER_SETUP_LIST(name)                                                                                       \
    MonsterSetupField                                                                                                  \
    {                                                                                                                  \
        #name, [](const MonsterSetup& setup) { return !setup.name.empty(); },                                          \
            [](const MonsterSetup& left, const MonsterSetup& right) { return left.name == right.name; },               \
            [](MonsterSetup& to, const MonsterSetup& from) { to.name = from.name; }                                    \
    }

const MonsterSetupField SetupFields[] = {
    {"scale", [](const MonsterSetup& setup) { return setup.scale.has_value() || setup.scaleByCharacter; },
     [](const MonsterSetup& left, const MonsterSetup& right)
     { return left.scale == right.scale && left.scaleByCharacter == right.scaleByCharacter; },
     [](MonsterSetup& to, const MonsterSetup& from)
     {
         to.scale = from.scale;
         to.scaleByCharacter = from.scaleByCharacter;
     }},
    MONSTER_SETUP_OPTIONAL(outline),
    MONSTER_SETUP_OPTIONAL(subType),
    MONSTER_SETUP_OPTIONAL(lookLevel),
    MONSTER_SETUP_OPTIONAL(glowMesh),
    MONSTER_SETUP_OPTIONAL(glowLight),
    MONSTER_SETUP_OPTIONAL(hiddenMesh),
    MONSTER_SETUP_OPTIONAL(shadow),
    MONSTER_SETUP_OPTIONAL(hideShadow),
    MONSTER_SETUP_OPTIONAL(alphaTarget),
    MONSTER_SETUP_OPTIONAL(light),
    MONSTER_SETUP_OPTIONAL(pkColor),
    MONSTER_SETUP_OPTIONAL(helperPlate),
    MONSTER_SETUP_OPTIONAL(selectable),
    {"class", [](const MonsterSetup& setup) { return setup.playerClass.has_value(); },
     [](const MonsterSetup& left, const MonsterSetup& right) { return left.playerClass == right.playerClass; },
     [](MonsterSetup& to, const MonsterSetup& from) { to.playerClass = from.playerClass; }},
    MONSTER_SETUP_OPTIONAL(classStep),
    {"equipment", [](const MonsterSetup& setup) { return !setup.equipment.IsEmpty(); },
     [](const MonsterSetup& left, const MonsterSetup& right) { return left.equipment == right.equipment; },
     [](MonsterSetup& to, const MonsterSetup& from) { to.equipment = from.equipment; }},
    MONSTER_SETUP_LIST(spawnEffects),
    MONSTER_SETUP_LIST(spawnBuffs),
    MONSTER_SETUP_OPTIONAL(spawnSound),
    MONSTER_SETUP_LIST(randomLook),
    {"randomStart", [](const MonsterSetup& setup) { return setup.randomStart; },
     [](const MonsterSetup& left, const MonsterSetup& right) { return left.randomStart == right.randomStart; },
     [](MonsterSetup& to, const MonsterSetup& from) { to.randomStart = from.randomStart; }},
    MONSTER_SETUP_OPTIONAL(positionOffset),
    MONSTER_SETUP_OPTIONAL(keepGroundHeight),
    MONSTER_SETUP_OPTIONAL(heightOffset),
    MONSTER_SETUP_OPTIONAL(height),
    MONSTER_SETUP_OPTIONAL(tileOffset),
    {"boundingBox", [](const MonsterSetup& setup) { return setup.boxMin.has_value() || setup.boxMax.has_value(); },
     [](const MonsterSetup& left, const MonsterSetup& right)
     { return left.boxMin == right.boxMin && left.boxMax == right.boxMax; },
     [](MonsterSetup& to, const MonsterSetup& from)
     {
         to.boxMin = from.boxMin;
         to.boxMax = from.boxMax;
     }},
    MONSTER_SETUP_OPTIONAL(startAction),
    MONSTER_SETUP_OPTIONAL(startFrame),
    MONSTER_SETUP_OPTIONAL(deathEffect),
    MONSTER_SETUP_OPTIONAL(live),
    MONSTER_SETUP_OPTIONAL(noTurnOnHit),
    MONSTER_SETUP_OPTIONAL(moveSpeed),
    MONSTER_SETUP_OPTIONAL(noBloodPool),
    MONSTER_SETUP_OPTIONAL(facing),
    MONSTER_SETUP_OPTIONAL(facingStep),
    MONSTER_SETUP_LIST(facingByTile),
    MONSTER_SETUP_OPTIONAL(appear),
    MONSTER_SETUP_OPTIONAL(teleport),
    MONSTER_SETUP_OPTIONAL(gate),
    MONSTER_SETUP_LIST(stateVariants),
    MONSTER_SETUP_LIST(hooks),
};

#undef MONSTER_SETUP_OPTIONAL
#undef MONSTER_SETUP_LIST

struct NamedValue
{
    std::string_view name;
    int value;
};

const NamedValue Buffs[] = {
    {"poison", eDeBuff_Poison},
};

const NamedValue PkColors[] = {
    {"hero2", PVP_HERO2},     {"hero1", PVP_HERO1},         {"neutral", PVP_NEUTRAL},
    {"caution", PVP_CAUTION}, {"murderer1", PVP_MURDERER1}, {"murderer2", PVP_MURDERER2},
};

std::optional<int> FindValue(std::span<const NamedValue> values, std::string_view name)
{
    const auto found = std::ranges::find(values, name, &NamedValue::name);
    return found != values.end() ? std::optional<int>(found->value) : std::nullopt;
}

std::string_view FindName(std::span<const NamedValue> values, int value)
{
    const auto found = std::ranges::find(values, value, &NamedValue::value);
    return found != values.end() ? found->name : std::string_view();
}
} // namespace

constexpr std::string_view RageBosses[] = {"Raymond", "Ercanne", "Daesuler", "Gallia",
                                           "Bermont", "Kato",    "Gaion",    "Jerint"};
constexpr std::string_view Storages[] = {"allied", "illusion"};

// The hooks left after Q-MN36 and Q-MN46 that act at spawn.
constexpr MonsterHookKind HookKinds[] = {
    // Lets Selupan drop down from where he waits.
    {"RaklionBossDrop", "", {}},
    // Kanturu's entrance window takes the Gateway Machine, which its
    // animations and the way in follow.
    {"Kanturu2ndEntrance", "", {}},
    // A new Empire Guardian boss starts without its predecessor's rage.
    {"EmpireGuardianRageReset", "boss", RageBosses},
    // A new storage of the Illusion Temple shows no point effect it had.
    {"IllusionTempleStorageReset", "storage", Storages},
};

// playerTransformed: the monster is made for a player transformed into it
// (0x45, Q-MN38d); crywolfReady: Crywolf's battle is about to start, its
// monsters come in at once (Q-MN38c).
constexpr std::string_view States[] = {"castleSiegeRunning", "castleSiegeNotRunning", "raklionBossWaiting",
                                       "playerTransformed", "crywolfReady"};

std::span<const std::string_view> GetMonsterStates()
{
    return States;
}

bool IsMonsterState(std::string_view name)
{
    return std::ranges::find(States, name) != std::end(States);
}

// The player bodies by the code's names: MODEL_BODY_HELM + n and so on.
struct BodyRange
{
    std::string_view symbol;
    int first;
};
constexpr BodyRange BodyRanges[] = {
    {"MODEL_BODY_HELM", MODEL_BODY_HELM},   {"MODEL_BODY_ARMOR", MODEL_BODY_ARMOR},
    {"MODEL_BODY_PANTS", MODEL_BODY_PANTS}, {"MODEL_BODY_GLOVES", MODEL_BODY_GLOVES},
    {"MODEL_BODY_BOOTS", MODEL_BODY_BOOTS},
};

std::optional<int> FindMonsterBodyPartSlot(std::string_view symbol)
{
    if (const std::optional<int> slot = FindMonsterModelSlot(symbol))
        return slot;
    for (const BodyRange& range : BodyRanges)
    {
        if (!symbol.starts_with(range.symbol))
            continue;
        const std::string_view rest = symbol.substr(range.symbol.size());
        if (rest.empty())
            return range.first;
        int offset = 0;
        const auto [end, error] = std::from_chars(rest.data() + 1, rest.data() + rest.size(), offset);
        if (rest.front() == '+' && error == std::errc() && end == rest.data() + rest.size() && offset >= 0 &&
            offset < MODEL_BODY_NUM)
            return range.first + offset;
    }
    return std::nullopt;
}

std::string GetMonsterBodyPartSymbol(int slot)
{
    if (const std::string_view symbol = GetMonsterModelSymbol(slot); !symbol.empty())
        return std::string(symbol);
    for (const BodyRange& range : BodyRanges)
    {
        if (slot >= range.first && slot < range.first + MODEL_BODY_NUM)
            return std::string(range.symbol) + (slot == range.first ? "" : "+" + std::to_string(slot - range.first));
    }
    return {};
}

std::optional<int> FindMountLook(std::string_view name)
{
    if (name.empty())
        return 0;
    if (name == "doppelganger")
        return 1;
    return std::nullopt;
}

bool IsDarkHorse(const MonsterItem& item)
{
    return item.part.empty() && Items::MakeItemType(item.group, item.number) == ITEM_DARK_HORSE_ITEM;
}

std::string_view MonsterHook::Value(std::string_view key) const
{
    const auto found = std::ranges::find(values, key, &std::pair<std::string, std::string>::first);
    return found != values.end() ? std::string_view(found->second) : std::string_view();
}

std::span<const MonsterHookKind> GetMonsterHookKinds()
{
    return HookKinds;
}

const MonsterHookKind* FindMonsterHookKind(std::string_view name)
{
    const auto found = std::ranges::find(HookKinds, name, &MonsterHookKind::name);
    return found != std::end(HookKinds) ? &*found : nullptr;
}

bool MonsterEquipment::IsEmpty() const
{
    return *this == MonsterEquipment{};
}

std::span<const MonsterSetupField> GetMonsterSetupFields()
{
    return SetupFields;
}

std::optional<int> FindMonsterBuff(std::string_view name)
{
    return FindValue(Buffs, name);
}

std::string_view GetMonsterBuffName(int buff)
{
    return FindName(Buffs, buff);
}

std::vector<std::string_view> GetMonsterBuffNames()
{
    std::vector<std::string_view> names;
    for (const NamedValue& buff : Buffs)
        names.push_back(buff.name);
    return names;
}

std::optional<int> FindPkColor(std::string_view name)
{
    return FindValue(PkColors, name);
}

std::string_view GetPkColorName(int pk)
{
    return FindName(PkColors, pk);
}
} // namespace Data::Monsters
