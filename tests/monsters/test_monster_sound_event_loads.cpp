#include "stdafx.h"

// The sounds a model's sound events play are loaded where they play (monsters
// design, MN7): by the game at start (OpenSounds), by the load of each map the
// events play on, or by the model itself, which loads the sounds its entry
// lists when it opens.

#include "MonsterModelRecorder.h"

#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
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

std::vector<int> WorldsOf(const Data::Monsters::MonsterModelDefinition& model)
{
    return model.soundEventWorlds.empty() ? std::vector<int>{WD_0LORENCIA} : model.soundEventWorlds;
}

// The numbers of the sounds the model's entry lists, which it loads when it
// opens.
std::set<int> OwnSounds(const Data::Monsters::MonsterModelDefinition& model)
{
    std::set<int> numbers;
    for (const Data::Monsters::MonsterModelSound& sound : model.sounds)
    {
        if (const std::optional<int> number =
                Data::Monsters::ModelLoader::FindSoundNumber(model, Data::Monsters::SoundNameOf(sound)))
        {
            numbers.insert(*number);
        }
    }
    return numbers;
}
} // namespace

TEST_CASE("Monster sounds: the sounds of the sound events are loaded on the maps they play on "
          "[monsters][sounds][models]")
{
    std::vector<Data::Monsters::MonsterModelDefinition> models;
    std::set<int> startup;
    {
        MonsterRecorder::LoadScope scope(WD_0LORENCIA);
        for (const Data::Monsters::MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
        {
            if (!model->soundEvents.empty())
            {
                models.push_back(*model);
            }
        }
        startup = LoadedIds(scope.Run([] { OpenSounds(); }));
    }
    REQUIRE(models.size() == 70);
    REQUIRE_FALSE(startup.empty());

    // Each map's load once.
    std::map<int, std::set<int>> mapLoads;
    for (const Data::Monsters::MonsterModelDefinition& model : models)
    {
        for (const int world : WorldsOf(model))
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

    // Each model loads the sounds its events play (P21, P22).
    std::set<std::string> missing;
    for (const Data::Monsters::MonsterModelDefinition& model : models)
    {
        const std::set<int> own = OwnSounds(model);
        for (const int world : WorldsOf(model))
        {
            const std::set<int>& map = mapLoads.at(world);
            for (const Data::Monsters::MonsterSoundEvent& event : model.soundEvents)
            {
                for (const std::string& sound : event.sounds)
                {
                    const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(model, sound);
                    REQUIRE(number.has_value());
                    if (!startup.contains(*number) && !map.contains(*number) && !own.contains(*number))
                    {
                        missing.insert(model.slot + " " + sound + " on " + std::to_string(world));
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

TEST_CASE("Monster sounds: a model loads the sounds its entry lists [monsters][sounds][models]")
{
    // What the test above takes from the entries: opening a model loads them.
    MonsterRecorder::LoadScope scope(WD_69EMPIREGUARDIAN1);
    const Data::Monsters::MonsterModelDefinition* raymond = g_MonsterModelDatabase.Find(MODEL_RAYMOND);
    REQUIRE(raymond != nullptr);
    const std::set<int> loaded = LoadedIds(scope.Run([] { Data::Monsters::ModelLoader::Open(MODEL_RAYMOND); }));
    CHECK(loaded == OwnSounds(*raymond));
}
