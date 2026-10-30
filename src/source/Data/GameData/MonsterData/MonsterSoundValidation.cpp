#include "stdafx.h"

#include "MonsterSoundValidation.h"

#include "Core/Globals/_enum.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundIds.h"
#include "Data/GameData/MonsterData/MonsterSoundsJson.h"

#include <algorithm>
#include <array>
#include <map>
#include <optional>
#include <set>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;

constexpr std::array<std::string_view, 2> ChaosCastleLooks = {"MONSTER_MODEL_CHAOSCASTLE_ELF",
                                                              "MONSTER_MODEL_CHAOSCASTLE_WIZARD"};
// The creator of the ranged Chaos Castle soldiers draws one of the two
// (ZzzCharacter.cpp, CreateMonster).
constexpr std::array<int, 7> ChaosCastleRanged = {
    MONSTER_CHAOS_CASTLE_2,  MONSTER_CHAOS_CASTLE_4,  MONSTER_CHAOS_CASTLE_6, MONSTER_CHAOS_CASTLE_8,
    MONSTER_CHAOS_CASTLE_10, MONSTER_CHAOS_CASTLE_12, MONSTER_CHAOS_CASTLE_14};

bool NamesSoundOfSet(const MonsterSoundSet& set, const std::string& value)
{
    return std::any_of(set.sounds.begin(), set.sounds.end(),
                       [&](const MonsterSound& sound) { return SoundNameOf(sound) == value; });
}

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& source,
              int monster, const std::string& field, const std::string& message)
{
    issues.push_back({severity, source, ItemDataIssue::NoItem, monster, field, message});
}

std::string MonsterField(int number, const std::string& key)
{
    return "monster " + std::to_string(number) + " " + key;
}

// The sets the monsters play, with each monster and the slot it plays it on.
struct SetUser
{
    int monster = 0;
    std::optional<int> slot;
};
} // namespace

void ValidateMonsterSoundSetNames(std::span<const MonsterSoundSet> sets, const std::string& source,
                                  std::vector<ItemDataIssue>& issues)
{
    for (const MonsterSoundSet& set : sets)
    {
        for (size_t i = 0; i < set.sounds.size(); ++i)
        {
            const std::string& id = set.sounds[i].id;
            if (!id.empty() && !FindMonsterSoundId(id))
            {
                AddIssue(issues, ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem,
                         MonsterSoundSetField(set.name, "sounds[" + std::to_string(i) + "].id"),
                         "\"" + id + "\" is not a SOUND_ id that code plays a monster sound by");
            }
        }
        for (size_t event = 0; event < set.soundEvents.size(); ++event)
        {
            const std::vector<std::string>& sounds = set.soundEvents[event].sounds;
            for (size_t i = 0; i < sounds.size(); ++i)
            {
                if (NamesSoundOfSet(set, sounds[i]) || FindMonsterSoundId(sounds[i]))
                {
                    continue;
                }
                AddIssue(issues, ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem,
                         MonsterSoundSetField(set.name, "soundEvents[" + std::to_string(event) + "].sounds[" +
                                                            std::to_string(i) + "]"),
                         "\"" + sounds[i] +
                             "\" is no sound of the set (its id, or its file when it has none) and no SOUND_ id");
            }
        }
    }
}

std::span<const std::string_view> GetDrawnLookModels(int number)
{
    if (std::find(ChaosCastleRanged.begin(), ChaosCastleRanged.end(), number) != ChaosCastleRanged.end())
    {
        return ChaosCastleLooks;
    }
    return {};
}

void ValidateMonsterSoundLinks(std::span<const MonsterDefinition> monsters,
                               std::span<const MonsterModelDefinition> models, std::span<const MonsterSoundSet> sets,
                               const std::string& monstersSource, const std::string& soundsSource,
                               std::vector<ItemDataIssue>& issues)
{
    std::map<std::string, const MonsterSoundSet*, std::less<>> byName;
    for (const MonsterSoundSet& set : sets)
    {
        byName.emplace(set.name, &set);
    }
    std::map<int, const MonsterModelDefinition*> bySlot;
    for (const MonsterModelDefinition& model : models)
    {
        if (const std::optional<int> slot = FindMonsterModelSlot(model.slot))
        {
            bySlot.emplace(*slot, &model);
        }
    }
    std::map<std::string, std::vector<SetUser>, std::less<>> users;
    const auto use = [&](const std::string& name, const std::string& field, int number, const std::string& model)
    {
        if (!byName.contains(name))
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, monstersSource, number, MonsterField(number, field),
                     "\"" + name + "\" is no sound set of Sounds.json; the monster plays no sounds there");
            return;
        }
        users[name].push_back({number, FindMonsterModelReference(model)});
    };
    for (const MonsterDefinition& monster : monsters)
    {
        if (!monster.soundSet.empty())
        {
            use(monster.soundSet, "soundSet", monster.number, monster.model);
        }
        const std::span<const std::string_view> drawn = GetDrawnLookModels(monster.number);
        for (size_t i = 0; i < monster.randomVariants.size(); ++i)
        {
            const MonsterRandomVariant& variant = monster.randomVariants[i];
            const std::string key = "randomVariants[" + std::to_string(i) + "]";
            if (std::find(drawn.begin(), drawn.end(), variant.model) == drawn.end())
            {
                AddIssue(issues, ItemDataIssueSeverity::Error, monstersSource, monster.number,
                         MonsterField(monster.number, key + ".model"),
                         "the game never shows the monster with " + variant.model +
                             ": its looks are drawn in code until the setup moves into data (MN4)");
            }
            if (variant.soundSet)
            {
                use(*variant.soundSet, key + ".soundSet", monster.number, variant.model);
            }
        }
    }
    for (const MonsterSoundSet& set : sets)
    {
        const auto found = users.find(set.name);
        if (found == users.end())
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, soundsSource, ItemDataIssue::NoItem,
                     MonsterSoundSetField(set.name, "name"), "no monster plays the set");
            continue;
        }
        for (size_t event = 0; event < set.soundEvents.size(); ++event)
        {
            for (const std::string& cue : set.soundEvents[event].cues)
            {
                std::set<int> lacking;
                for (const SetUser& user : found->second)
                {
                    const auto model = user.slot ? bySlot.find(*user.slot) : bySlot.end();
                    const bool has =
                        model != bySlot.end() &&
                        std::any_of(model->second->cues.begin(), model->second->cues.end(),
                                    [&](const MonsterModelCue& candidate) { return candidate.name == cue; });
                    if (!has)
                    {
                        lacking.insert(user.monster);
                    }
                }
                if (lacking.empty())
                {
                    continue;
                }
                std::string numbers;
                for (const int number : lacking)
                {
                    numbers += (numbers.empty() ? "" : ", ") + std::to_string(number);
                }
                AddIssue(issues, ItemDataIssueSeverity::Warning, soundsSource, ItemDataIssue::NoItem,
                         MonsterSoundSetField(set.name, "soundEvents[" + std::to_string(event) + "].cues"),
                         "the model of monster " + numbers + " has no cue \"" + cue +
                             "\": the event never plays for it");
            }
        }
    }
}
} // namespace Data::Monsters
