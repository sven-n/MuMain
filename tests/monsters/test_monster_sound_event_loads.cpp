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
    // As in the original client: these models play sounds that another model
    // or another map loads, so they are silent unless that one was there.
    const std::set<std::string> known = {
        "MONSTER_MODEL_BURNING_LAVA_GIANT SOUND_PKFIELD_BURNINGLAVAGOLEM_ATTACK01",
        "MONSTER_MODEL_BURNING_LAVA_GIANT SOUND_PKFIELD_BURNINGLAVAGOLEM_ATTACK02",
        "MONSTER_MODEL_BURNING_LAVA_GIANT SOUND_PKFIELD_BURNINGLAVAGOLEM_DAMAGE01",
        "MONSTER_MODEL_BURNING_LAVA_GIANT SOUND_PKFIELD_BURNINGLAVAGOLEM_DEATH",
        "MONSTER_MODEL_BURNING_LAVA_GIANT SOUND_PKFIELD_BURNINGLAVAGOLEM_MOVE01",
        "MONSTER_MODEL_COOLUTIN SOUND_MONSTER_HELLSPIDERDIE",
        "MONSTER_MODEL_CRUEL_BLOOD_ASSASSIN SOUND_PKFIELD_BLOODASSASSIN_ATTACK",
        "MONSTER_MODEL_CRUEL_BLOOD_ASSASSIN SOUND_PKFIELD_BLOODASSASSIN_DAMAGE01",
        "MONSTER_MODEL_CRUEL_BLOOD_ASSASSIN SOUND_PKFIELD_BLOODASSASSIN_DEDTH",
        "MONSTER_MODEL_CRUEL_BLOOD_ASSASSIN SOUND_PKFIELD_BLOODASSASSIN_MOVE01",
        "MONSTER_MODEL_CRYPOS SOUND_KARUTAN_CRYPTA_DEATH",
        "MONSTER_MODEL_DARK_COOLUTIN SOUND_MONSTER_HELLSPIDERDIE",
        "MONSTER_MODEL_DARK_COOLUTIN SOUND_RAKLION_COOLERTIN_ATTACK",
        "MONSTER_MODEL_DARK_COOLUTIN SOUND_RAKLION_COOLERTIN_MOVE",
        "MONSTER_MODEL_DARK_GIANT SOUND_RAKLION_ICEGIANT_DEATH",
        "MONSTER_MODEL_DARK_GIANT SOUND_RAKLION_ICEGIANT_MOVE",
        "MONSTER_MODEL_DARK_IRON_KNIGHT SOUND_MONSTER_DEATH1",
        "MONSTER_MODEL_DARK_IRON_KNIGHT SOUND_RAKLION_IRON_KNIGHT_ATTACK",
        "MONSTER_MODEL_DARK_IRON_KNIGHT SOUND_RAKLION_IRON_KNIGHT_MOVE",
        "MONSTER_MODEL_DARK_MAMMOTH SOUND_RAKLION_GIANT_MAMUD_ATTACK",
        "MONSTER_MODEL_DARK_MAMMOTH SOUND_RAKLION_GIANT_MAMUD_DEATH",
        "MONSTER_MODEL_DARK_MAMMOTH SOUND_RAKLION_GIANT_MAMUD_MOVE",
        "MONSTER_MODEL_FORSAKER SOUND_RAKLION_SERUFAN_CURE",
        "MONSTER_MODEL_HEAD_MOUNTER SOUND_RAKLION_GIANT_MAMUD_DEATH",
        "MONSTER_MODEL_ICE_NAPIN SOUND_SWAMPOFQUIET_BLAZE_NAIPIN_BREATH01",
        "MONSTER_MODEL_ICE_NAPIN SOUND_SWAMPOFQUIET_NAIPIN_ATTACK01",
        "MONSTER_MODEL_ICE_NAPIN SOUND_SWAMPOFQUIET_NAIPIN_DEATH01",
        "MONSTER_MODEL_SAPI_QUEEN SOUND_SWAMPOFQUIET_SAPI_DEATH01",
        "MONSTER_MODEL_SAPI_QUEEN SOUND_SWAMPOFQUIET_SAPI_UNUS_ATTACK01",
        "MONSTER_MODEL_SHADOW_MASTER SOUND_SWAMPOFQUIET_SHADOW_DEATH01",
        "MONSTER_MODEL_SHADOW_MASTER SOUND_SWAMPOFQUIET_SHADOW_ROOK_ATTACK01",
    };
    for (const std::string& line : missing)
    {
        INFO(line);
        CHECK(known.contains(line));
    }
    for (const std::string& line : known)
    {
        INFO(line);
        CHECK(missing.contains(line));
    }
}
