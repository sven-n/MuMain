#include "stdafx.h"

// The sounds a set's sound events of animations and cues play are loaded
// where they play (monsters design, MN7, Q-MN78): by the game at start
// (OpenSounds), by the load of each map the events play on, or by the model
// of a monster that plays the set, which loads the sounds of its monsters'
// sets when it opens. The events at moments, which were the sound slots, play
// a sound any set loads (test_monster_sound_files.cpp).

#include "MonsterModelRecorder.h"

#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "Engine/Object/ZzzOpenData.h"
#include "World/MapInfra/MapManager.h"

#include <doctest.h>

#include <map>
#include <set>
#include <string>
#include <vector>

void OpenSounds();

namespace
{
std::set<int> LoadedIds(const MonsterRecorder::LoadChanges& changes)
{
    std::set<int> ids;
    for (const MonsterRecorder::SoundEvent& sound : changes.sounds)
    {
        if (!sound.release)
        {
            ids.insert(sound.id);
        }
    }
    return ids;
}

std::vector<int> WorldsOf(const Data::Monsters::MonsterSoundSet& set)
{
    if (set.maps.empty())
    {
        return {WD_0LORENCIA};
    }
    std::vector<int> maps;
    for (const Data::Maps::MapRef& ref : set.maps)
    {
        maps.push_back(ref.map);
    }
    return maps;
}

// The numbers of the sounds the set lists, which a model of its monsters
// loads when it opens.
std::set<int> OwnSounds(const Data::Monsters::MonsterSoundSet& set)
{
    std::set<int> numbers;
    for (const Data::Monsters::MonsterSound& sound : set.sounds)
    {
        if (const std::optional<int> number =
                Data::Monsters::ModelLoader::FindSoundNumber(set, Data::Monsters::SoundNameOf(sound)))
        {
            numbers.insert(*number);
        }
    }
    return numbers;
}

const Data::Monsters::MonsterSoundSet& SetNamed(const char* name)
{
    const Data::Monsters::MonsterSoundSet* set = g_MonsterSoundSetDatabase.Find(name);
    REQUIRE(set != nullptr);
    return *set;
}
} // namespace

TEST_CASE("Monster sounds: the sounds of the sound events are loaded on the maps they play on "
          "[monsters][sounds][models]")
{
    std::vector<Data::Monsters::MonsterSoundSet> sets;
    std::set<int> startup;
    {
        MonsterRecorder::LoadScope scope(WD_0LORENCIA);
        for (const Data::Monsters::MonsterSoundSet* set : g_MonsterSoundSetDatabase.GetAll())
        {
            Data::Monsters::MonsterSoundSet ofAnimations = *set;
            std::erase_if(ofAnimations.soundEvents, [](const Data::Monsters::MonsterSoundEvent& event)
                          { return event.animations.empty() && event.cues.empty(); });
            if (!ofAnimations.soundEvents.empty())
            {
                sets.push_back(std::move(ofAnimations));
            }
        }
        startup = LoadedIds(scope.Run([] { OpenSounds(); }));
    }
    // The 70 models whose events played in animations, as sets.
    REQUIRE(sets.size() >= 60);
    REQUIRE_FALSE(startup.empty());

    // Each map's load once.
    std::map<int, std::set<int>> mapLoads;
    for (const Data::Monsters::MonsterSoundSet& set : sets)
    {
        for (const int world : WorldsOf(set))
        {
            if (mapLoads.contains(world))
            {
                continue;
            }
            // Raklion's load needs the terrain (CreateObject) and cannot run
            // here; it loads no sound of its monsters, which their models load.
            if (world == WD_57ICECITY || world == WD_58ICECITY_BOSS)
            {
                mapLoads[world] = {};
                continue;
            }
            MonsterRecorder::LoadScope scope(world);
            mapLoads[world] = LoadedIds(scope.Run([] { gMapManager.Load(); }));
        }
    }

    // Each set loads the sounds its events play (P21, P22).
    std::set<std::string> missing;
    for (const Data::Monsters::MonsterSoundSet& set : sets)
    {
        const std::set<int> own = OwnSounds(set);
        for (const int world : WorldsOf(set))
        {
            const std::set<int>& map = mapLoads.at(world);
            for (const Data::Monsters::MonsterSoundEvent& event : set.soundEvents)
            {
                for (const std::string& sound : event.sounds)
                {
                    const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(set, sound);
                    REQUIRE(number.has_value());
                    if (!startup.contains(*number) && !map.contains(*number) && !own.contains(*number))
                    {
                        missing.insert(set.name + " " + sound + " on " + std::to_string(world));
                    }
                }
            }
        }
    }
    for (const std::string& line : missing)
    {
        MESSAGE(line);
    }
    CHECK(missing.empty());
}

TEST_CASE("Monster sounds: a model loads the sounds of the sets its monsters play (Q-MN78) [monsters][sounds][models]")
{
    // What the test above takes from the sets: opening a model loads them.
    MonsterRecorder::LoadScope scope(WD_69EMPIREGUARDIAN1);
    std::set<int> loaded = LoadedIds(scope.Run([] { Data::Monsters::ModelLoader::Open(MODEL_RAYMOND); }));
    CHECK(loaded == OwnSounds(SetNamed("Raymond")));

    // The model of the Rogue and the Death Centurion loads both their sets,
    // the ten files it loaded with the second set of SubType 9.
    loaded = LoadedIds(scope.Run([] { Data::Monsters::ModelLoader::Open(MODEL_DEATH_CENTURION); }));
    std::set<int> both = OwnSounds(SetNamed("Rogue Centurion"));
    const std::set<int> second = OwnSounds(SetNamed("Death Centurion"));
    both.insert(second.begin(), second.end());
    CHECK(both.size() == 10);
    CHECK(loaded == both);

    // The Chaos Castle wizard look's model loads the set of the random variant
    // that names it (Q-MN83); no monster's entry names the model.
    loaded = LoadedIds(scope.Run([] { Data::Monsters::ModelLoader::Open(MODEL_CHAOS_CASTLE_WIZARD); }));
    CHECK(loaded == OwnSounds(SetNamed("Chaos Castle Wizard")));
    CHECK(Data::Monsters::ModelLoader::FindSoundSetsOfSlot(MODEL_CHAOS_CASTLE_WIZARD).size() == 1);
}
