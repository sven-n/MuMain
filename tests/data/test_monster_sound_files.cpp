#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Core/Globals/_enum.h"
#include "Data/DataHandler/EffectData/EffectDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterSoundJsonStorage.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundValidation.h"
#include "Data/GameData/MonsterData/MonsterSoundsJson.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

using Data::Items::HasFatal;
using Data::Items::ItemDataIssue;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterSound;
using Data::Monsters::MonsterSoundSet;

namespace
{
const std::filesystem::path ClientData = MU_TEST_DATA_DIR;

std::string Messages(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}

std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

// The shipped lists, read as the game reads them.
struct Shipped
{
    std::vector<MonsterDefinition> monsters;
    std::vector<MonsterModelDefinition> models;
    Data::Monsters::MonsterSoundsLoadResult sounds;
};

Shipped LoadShipped()
{
    ClientDataScope scope(ClientData.parent_path());
    std::string error;
    REQUIRE(Data::Effects::LoadEffectTypes(error));
    Shipped shipped;
    shipped.monsters = Data::Monsters::LoadMonstersFile(Data::Monsters::GetMonstersFile()).monsters;
    shipped.models = Data::Monsters::LoadMonsterModelsFile(Data::Monsters::GetMonsterModelsFile()).models;
    shipped.sounds = Data::Monsters::LoadMonsterSoundsFile(Data::Monsters::GetMonsterSoundsFile());
    return shipped;
}

const MonsterDefinition& Find(const std::vector<MonsterDefinition>& monsters, int number)
{
    const auto found = std::find_if(monsters.begin(), monsters.end(),
                                    [&](const MonsterDefinition& monster) { return monster.number == number; });
    REQUIRE(found != monsters.end());
    return *found;
}
} // namespace

TEST_CASE("Monster sounds: the shipped sound sets load, and what the lists name of each other is there (Q-MN78) "
          "[data][monsters]")
{
    const Shipped shipped = LoadShipped();
    INFO(Messages(shipped.sounds.issues));
    CHECK(shipped.sounds.issues.empty());
    CHECK(shipped.sounds.sets.size() > 100);
    std::vector<ItemDataIssue> issues;
    Data::Monsters::ValidateMonsterSoundLinks(shipped.monsters, shipped.models, shipped.sounds.sets, "Monsters.json",
                                              "Sounds.json", issues);
    INFO(Messages(issues));
    CHECK(issues.empty());
}

TEST_CASE("Monster sounds: the shipped sound sets are written as their writer writes them [data][monsters]")
{
    const std::optional<std::string> text = Data::ReadTextFile(ClientData / "Monsters" / "Sounds.json");
    REQUIRE(text);
    std::vector<MonsterSoundSet> sets;
    std::vector<ItemDataIssue> issues;
    Data::Monsters::ReadMonsterSoundsJson(*text, "Sounds.json", sets, issues);
    REQUIRE_FALSE(HasFatal(issues));
    CHECK(Data::Monsters::WriteMonsterSoundsJson(sets) == *text);
}

TEST_CASE("Monster sounds: the second sets of SubType 9 are the monsters' own (Q-MN78) [data][monsters]")
{
    const Shipped shipped = LoadShipped();
    for (const int number : {145, 175, 183, 191, 261, 269, 336})
    {
        INFO(number);
        CHECK(Find(shipped.monsters, number).soundSet == "Death Centurion");
    }
    for (const int number : {148, 178, 186, 194, 264, 272, 332})
    {
        INFO(number);
        CHECK(Find(shipped.monsters, number).soundSet == "Rogue Centurion");
    }
    for (const int number : {161, 181, 189, 197, 267, 338})
    {
        INFO(number);
        CHECK(Find(shipped.monsters, number).soundSet == "Illusion of Kundun");
    }
    for (const int number : {160, 180, 188, 196, 266, 274, 337})
    {
        INFO(number);
        CHECK(Find(shipped.monsters, number).soundSet == "Schriker");
    }
    // No set plays by SubType any more: the four sets have different files.
    std::set<std::string> files;
    for (const MonsterSoundSet& set : shipped.sounds.sets)
    {
        if (set.name == "Death Centurion" || set.name == "Rogue Centurion" || set.name == "Illusion of Kundun" ||
            set.name == "Schriker")
        {
            for (const MonsterSound& sound : set.sounds)
            {
                CHECK(files.insert(Lower(sound.file)).second);
            }
        }
    }
    CHECK(files.size() == 20);
}

TEST_CASE("Monster sounds: the ranged Chaos Castle soldiers' wizard look plays its own set (Q-MN83) "
          "[data][monsters]")
{
    const Shipped shipped = LoadShipped();
    for (const int number : {163, 165, 167, 169, 171, 173, 427})
    {
        INFO(number);
        const MonsterDefinition& monster = Find(shipped.monsters, number);
        CHECK(monster.soundSet == "Chaos Castle");
        REQUIRE(monster.randomVariants.size() == 1);
        CHECK(monster.randomVariants[0].model == "MONSTER_MODEL_CHAOSCASTLE_WIZARD");
        CHECK(monster.randomVariants[0].soundSet == "Chaos Castle Wizard");
    }
    CHECK(Find(shipped.monsters, 162).randomVariants.empty());
}

TEST_CASE("Monster sounds: the steps of the shipped models are cues, which their sets play on (Q-MN79) "
          "[data][monsters]")
{
    const Shipped shipped = LoadShipped();
    std::vector<std::string> withCues;
    for (const MonsterModelDefinition& model : shipped.models)
    {
        if (model.cues.empty())
        {
            continue;
        }
        withCues.push_back(model.slot);
        REQUIRE(model.cues.size() == 2);
        CHECK(model.cues[0].name == "step 1");
        CHECK(model.cues[0].frames == std::pair<float, float>{1.0f, 2.0f});
        CHECK(model.cues[1].name == "step 2");
        CHECK(model.cues[1].frames == std::pair<float, float>{7.0f, 8.0f});
    }
    CHECK(withCues.size() == 12);
    CHECK(std::find(withCues.begin(), withCues.end(), "MONSTER_MODEL_RAYMOND") != withCues.end());
}

TEST_CASE("Monster sounds: a SOUND_ id that a sound event at moments names is loaded by a set of the shipped "
          "list [data][monsters]")
{
    // The events that were sound slots (MNS) play the sound of its number,
    // whichever set loaded it: the Giant Ogre's attack plays the Dark Skull
    // Soldier's attack sound in Kalima, Vepar's the Stone Golem's death sound.
    // A number that no set loads plays nothing; the slots that named one in
    // the original client name their models' sounds (P9, Q-MN20).
    const Shipped shipped = LoadShipped();
    std::set<std::string> loaded;
    for (const MonsterSoundSet& set : shipped.sounds.sets)
    {
        for (const MonsterSound& sound : set.sounds)
        {
            if (!sound.id.empty())
            {
                loaded.insert(sound.id);
            }
        }
    }
    std::set<std::string> notLoaded;
    for (const MonsterSoundSet& set : shipped.sounds.sets)
    {
        for (const Data::Monsters::MonsterSoundEvent& event : set.soundEvents)
        {
            if (event.moments.empty())
            {
                continue;
            }
            for (const std::string& value : event.sounds)
            {
                if (value.starts_with("SOUND_") && !loaded.contains(value))
                {
                    notLoaded.insert(value);
                }
            }
        }
    }
    std::string listed;
    for (const std::string& id : notLoaded)
    {
        listed += id + " ";
    }
    INFO("named by events at moments, loaded by no set: " << listed);
    CHECK(notLoaded.empty());
}

// A sound that several sets list is loaded once (MonsterModelLoader skips one
// already loaded from the same file): they must list the same file, channels
// and 3D setting, or the later set loads it again with its own.
TEST_CASE("Monster sounds: sets that share a sound id load it alike [data][monsters]")
{
    const Shipped shipped = LoadShipped();
    REQUIRE_FALSE(HasFatal(shipped.sounds.issues));
    std::map<std::string, std::set<std::string>> loads;
    for (const MonsterSoundSet& set : shipped.sounds.sets)
    {
        for (const MonsterSound& sound : set.sounds)
        {
            if (!sound.id.empty())
            {
                loads[sound.id].insert(Lower(sound.file) + " channels " + std::to_string(sound.channels) +
                                       (sound.spatial ? " 3d" : ""));
            }
        }
    }
    std::vector<std::string> differing;
    for (const auto& [id, ways] : loads)
    {
        if (ways.size() > 1)
        {
            differing.push_back(id);
        }
    }
    // As in the original client: the Beam Knight loads death1.wav with two
    // channels in 3D, the Iron Knight (and its dark twin) with one.
    CHECK(differing == std::vector<std::string>{"SOUND_MONSTER_DEATH1"});
}

TEST_CASE("Monster sounds: the game's load fills the sound set table [data][monsters]")
{
    ClientDataScope scope(ClientData.parent_path());
    std::string error;
    REQUIRE(Data::Effects::LoadEffectTypes(error));
    REQUIRE(g_MonsterDataHandler.Load(error));
    REQUIRE(g_MonsterDataHandler.LoadModels(error));
    REQUIRE_MESSAGE(g_MonsterDataHandler.LoadSounds(error), error);
    const MonsterSoundSet* raymond = g_MonsterSoundSetDatabase.Find("Raymond");
    REQUIRE(raymond != nullptr);
    CHECK_FALSE(raymond->sounds.empty());
    CHECK(g_MonsterSoundSetDatabase.Find("Nobody") == nullptr);
}
