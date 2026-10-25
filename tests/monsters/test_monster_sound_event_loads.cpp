#include "stdafx.h"

// The sounds a model's sound events play are loaded where they play (monsters
// design, MN7): by the game at start (OpenSounds), by the load of each map the
// events play on, or by the model itself when it opens.

#include "MonsterModelRecorder.h"

#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Engine/Object/ZzzOpenData.h"
#include "World/MapInfra/MapManager.h"

#include <doctest.h>

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
} // namespace

TEST_CASE("Monster sounds: the sounds of the sound events are loaded on the maps they play on "
          "[monsters][sounds][models]")
{
    std::vector<Data::Monsters::MonsterModelDefinition> models;
    {
        MonsterRecorder::LoadScope table(0);
        for (const Data::Monsters::MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
        {
            if (!model->soundEvents.empty())
            {
                models.push_back(*model);
            }
        }
    }
    REQUIRE(models.size() == 70);

    std::set<std::string> missing;
    for (const Data::Monsters::MonsterModelDefinition& model : models)
    {
        const int slot = *Data::Monsters::FindMonsterModelSlot(model.slot);
        const std::vector<int> worlds =
            model.soundEventWorlds.empty() ? std::vector<int>{WD_0LORENCIA} : model.soundEventWorlds;
        for (const int world : worlds)
        {
            MonsterRecorder::LoadScope scope(world);
            std::set<int> loaded = LoadedIds(scope.Run([] { OpenSounds(); }));
            // Raklion's load needs the terrain (CreateObject) and cannot run
            // here; it loads no sound of its monsters, which their models load.
            const bool raklion = world == WD_57ICECITY || world == WD_58ICECITY_BOSS;
            const std::set<int> map = raklion ? std::set<int>{} : LoadedIds(scope.Run([] { gMapManager.Load(); }));
            const std::set<int> own = LoadedIds(scope.Run([&] { Data::Monsters::ModelLoader::Open(slot); }));
            loaded.insert(map.begin(), map.end());
            loaded.insert(own.begin(), own.end());
            for (const Data::Monsters::MonsterSoundEvent& event : model.soundEvents)
            {
                for (const std::string& sound : event.sounds)
                {
                    const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(model, sound);
                    REQUIRE(number.has_value());
                    if (!loaded.contains(*number))
                    {
                        missing.insert(model.slot + " " + sound);
                    }
                }
            }
        }
    }
    // Each model loads the sounds its events play (P21, P22).
    for (const std::string& line : missing)
    {
        MESSAGE(line);
    }
    CHECK(missing.empty());
}
