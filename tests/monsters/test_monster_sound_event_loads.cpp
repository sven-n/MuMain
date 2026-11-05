#include "stdafx.h"

// The sounds a set's sound events of animations and cues play are loaded
// where they play (monsters design, MN7, Q-MN78): by the game at start
// (OpenSounds), by the load of each map the events play on, or by the model
// of a monster that plays the set, which loads the sounds of its monsters'
// sets when it opens. The events at moments, which were the sound slots, play
// a sound any set loads (test_monster_sound_files.cpp).

#include "MonsterModelRecorder.h"

#include "Audio/DSPlaySound.h"
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

TEST_CASE("Monster sounds: the Aida monsters' models load the sounds the Aida map loaded [monsters][sounds][models]")
{
    // What CMapManager::Load loaded for Aida before the monsters' sets, all
    // with one channel and no 3D (and eElec1 and eElec2 again, which
    // OpenSounds loads at start). The map named the Death Rider's first idle
    // dr_idle1.wav, which no client has: the sound never loaded.
    const std::map<int, std::wstring> before = {
        {SOUND_AIDA_BLUEGOLEM_MOVE1, L"Data\\Sound\\w34\\bg_idle1.wav"},
        {SOUND_AIDA_BLUEGOLEM_MOVE2, L"Data\\Sound\\w34\\bg_idle2.wav"},
        {SOUND_AIDA_BLUEGOLEM_ATTACK1, L"Data\\Sound\\w34\\bg_attack1.wav"},
        {SOUND_AIDA_BLUEGOLEM_ATTACK2, L"Data\\Sound\\w34\\bg_attack2.wav"},
        {SOUND_AIDA_BLUEGOLEM_DIE, L"Data\\Sound\\w34\\bg_death.wav"},
        {SOUND_AIDA_DEATHRAIDER_MOVE1, L"Data\\Sound\\w34\\dr_idle1.wav"},
        {SOUND_AIDA_DEATHRAIDER_MOVE2, L"Data\\Sound\\w34\\dr_idle2.wav"},
        {SOUND_AIDA_DEATHRAIDER_ATTACK1, L"Data\\Sound\\w34\\dr_attack1.wav"},
        {SOUND_AIDA_DEATHRAIDER_ATTACK2, L"Data\\Sound\\w34\\dr_attack2.wav"},
        {SOUND_AIDA_DEATHRAIDER_DIE, L"Data\\Sound\\w34\\dr_death.wav"},
        {SOUND_AIDA_DEATHTREE_MOVE1, L"Data\\Sound\\w34\\dt_idle1.wav"},
        {SOUND_AIDA_DEATHTREE_MOVE2, L"Data\\Sound\\w34\\dt_idle2.wav"},
        {SOUND_AIDA_DEATHTREE_ATTACK1, L"Data\\Sound\\w34\\dt_attack1.wav"},
        {SOUND_AIDA_DEATHTREE_ATTACK2, L"Data\\Sound\\w34\\dt_attack2.wav"},
        {SOUND_AIDA_DEATHTREE_DIE, L"Data\\Sound\\w34\\dt_death.wav"},
        {SOUND_AIDA_FORESTORC_MOVE1, L"Data\\Sound\\w34\\fo_idle1.wav"},
        {SOUND_AIDA_FORESTORC_MOVE2, L"Data\\Sound\\w34\\fo_idle2.wav"},
        {SOUND_AIDA_FORESTORC_ATTACK1, L"Data\\Sound\\w34\\fo_attack1.wav"},
        {SOUND_AIDA_FORESTORC_ATTACK2, L"Data\\Sound\\w34\\fo_attack2.wav"},
        {SOUND_AIDA_FORESTORC_DIE, L"Data\\Sound\\w34\\fo_death.wav"},
        {SOUND_AIDA_HELL_MOVE1, L"Data\\Sound\\w34\\hm_idle1.wav"},
        {SOUND_AIDA_HELL_MOVE2, L"Data\\Sound\\w34\\hm_idle2.wav"},
        {SOUND_AIDA_HELL_ATTACK1, L"Data\\Sound\\w34\\hm_attack1.wav"},
        {SOUND_AIDA_HELL_ATTACK2, L"Data\\Sound\\w34\\hm_firelay.wav"},
        {SOUND_AIDA_HELL_ATTACK3, L"Data\\Sound\\w34\\hm_bloodywind.wav"},
        {SOUND_AIDA_HELL_DIE, L"Data\\Sound\\w34\\hm_death.wav"},
        {SOUND_AIDA_WITCHQUEEN_MOVE1, L"Data\\Sound\\w34\\wq_idle1.wav"},
        {SOUND_AIDA_WITCHQUEEN_MOVE2, L"Data\\Sound\\w34\\wq_idle2.wav"},
        {SOUND_AIDA_WITCHQUEEN_ATTACK1, L"Data\\Sound\\w34\\wq_attack1.wav"},
        {SOUND_AIDA_WITCHQUEEN_ATTACK2, L"Data\\Sound\\w34\\wq_attack2.wav"},
        {SOUND_AIDA_WITCHQUEEN_DIE, L"Data\\Sound\\w34\\wq_death.wav"},
    };
    // The models and the first of the sounds their map's code plays
    // (M33Aida::UpdateAidaMonsterEffects): five each, six for the Hell Maine.
    struct ModelSounds
    {
        int slot = 0;
        int first = 0;
        int count = 0;
    };
    const ModelSounds models[] = {
        {MODEL_WITCH_QUEEN, SOUND_AIDA_WITCHQUEEN_MOVE1, 5},
        {MODEL_BLOODY_WITCH_QUEEN, SOUND_AIDA_WITCHQUEEN_MOVE1, 5},
        {MODEL_GOLDEN_STONE_GOLEM, SOUND_AIDA_BLUEGOLEM_MOVE1, 5},
        {MODEL_BLOODY_GOLEM, SOUND_AIDA_BLUEGOLEM_MOVE1, 5},
        {MODEL_DEATH_RIDER, SOUND_AIDA_DEATHRAIDER_MOVE1, 5},
        {MODEL_BLOODY_DEATH_RIDER, SOUND_AIDA_DEATHRAIDER_MOVE1, 5},
        {MODEL_FOREST_ORC, SOUND_AIDA_FORESTORC_MOVE1, 5},
        {MODEL_BLOODY_ORC, SOUND_AIDA_FORESTORC_MOVE1, 5},
        {MODEL_DEATH_TREE, SOUND_AIDA_DEATHTREE_MOVE1, 5},
        {MODEL_HELL_MAINE, SOUND_AIDA_HELL_MOVE1, 6},
    };

    MonsterRecorder::LoadScope scope(WD_33AIDA);
    // The map loads only its ambient sound now.
    std::map<int, std::wstring> mapLoads;
    for (const MonsterRecorder::SoundEvent& sound : scope.Run([] { gMapManager.Load(); }).sounds)
    {
        if (!sound.release)
        {
            mapLoads[sound.id] = sound.file;
        }
    }
    CHECK(mapLoads == std::map<int, std::wstring>{{SOUND_AIDA_AMBIENT, L"Data\\Sound\\w34\\aida_ambi.wav"}});

    // Each model loads the sounds its code plays, as the map did.
    std::map<int, std::wstring> modelLoads;
    for (const ModelSounds& model : models)
    {
        INFO("model " << model.slot);
        std::set<int> numbers;
        for (const Data::Monsters::MonsterSoundSet* set : Data::Monsters::ModelLoader::FindSoundSetsOfSlot(model.slot))
        {
            const std::set<int> own = OwnSounds(*set);
            numbers.insert(own.begin(), own.end());
        }
        std::set<int> played;
        for (int sound = model.first; sound < model.first + model.count; ++sound)
        {
            played.insert(sound);
        }
        CHECK(numbers == played);
        const int slot = model.slot;
        for (const MonsterRecorder::SoundEvent& sound :
             scope.Run([slot] { Data::Monsters::ModelLoader::Open(slot); }).sounds)
        {
            if (!sound.release)
            {
                CHECK(sound.channels == 1);
                CHECK_FALSE(sound.spatial);
                modelLoads[sound.id] = sound.file;
            }
        }
    }
    std::map<int, std::wstring> expected = before;
    expected[SOUND_AIDA_DEATHRAIDER_MOVE1] = L"Data\\Sound\\w34\\dr_idle01.wav";
    CHECK(modelLoads == expected);
}

TEST_CASE("Monster sounds: the Kanturu monsters' models load the sounds Kanturu Ruins and Relics loaded (MNK) "
          "[monsters][sounds][models]")
{
    // What CMapManager::Load loaded for the two maps' monsters and the
    // gateway machine before their sets, all with one channel and no 3D.
    const std::map<int, std::wstring> before = {
        {SOUND_KANTURU_1ST_BER_MOVE1, L"Data\\Sound\\w37\\ber_idle-01.wav"},
        {SOUND_KANTURU_1ST_BER_MOVE2, L"Data\\Sound\\w37\\ber_idle-02.wav"},
        {SOUND_KANTURU_1ST_BER_ATTACK1, L"Data\\Sound\\w37\\ber_attack-01.wav"},
        {SOUND_KANTURU_1ST_BER_ATTACK2, L"Data\\Sound\\w37\\ber_attack-02.wav"},
        {SOUND_KANTURU_1ST_BER_DIE, L"Data\\Sound\\w37\\ber_death.wav"},
        {SOUND_KANTURU_1ST_GIGAN_MOVE1, L"Data\\Sound\\w37\\gigan_idle-01.wav"},
        {SOUND_KANTURU_1ST_GIGAN_ATTACK1, L"Data\\Sound\\w37\\gigan_attack-01.wav"},
        {SOUND_KANTURU_1ST_GIGAN_ATTACK2, L"Data\\Sound\\w37\\gigan_attack-02.wav"},
        {SOUND_KANTURU_1ST_GIGAN_DIE, L"Data\\Sound\\w37\\gigan_death.wav"},
        {SOUND_KANTURU_1ST_GENO_MOVE1, L"Data\\Sound\\w37\\geno_idle-01.wav"},
        {SOUND_KANTURU_1ST_GENO_MOVE2, L"Data\\Sound\\w37\\geno_idle-02.wav"},
        {SOUND_KANTURU_1ST_GENO_ATTACK1, L"Data\\Sound\\w37\\geno_attack-01.wav"},
        {SOUND_KANTURU_1ST_GENO_ATTACK2, L"Data\\Sound\\w37\\geno_attack-02.wav"},
        {SOUND_KANTURU_1ST_GENO_DIE, L"Data\\Sound\\w37\\geno_death.wav"},
        {SOUND_KANTURU_1ST_KENTA_MOVE1, L"Data\\Sound\\w37\\kenta_idle-01.wav"},
        {SOUND_KANTURU_1ST_KENTA_MOVE2, L"Data\\Sound\\w37\\kenta_idle-02.wav"},
        {SOUND_KANTURU_1ST_KENTA_ATTACK1, L"Data\\Sound\\w37\\kenta_attack-01.wav"},
        {SOUND_KANTURU_1ST_KENTA_ATTACK2, L"Data\\Sound\\w37\\kenta_skill-01.wav"},
        {SOUND_KANTURU_1ST_KENTA_DIE, L"Data\\Sound\\w37\\kenta_death.wav"},
        {SOUND_KANTURU_1ST_BLADE_MOVE1, L"Data\\Sound\\w37\\blade_idle-01.wav"},
        {SOUND_KANTURU_1ST_BLADE_MOVE2, L"Data\\Sound\\w37\\blade_idle-02.wav"},
        {SOUND_KANTURU_1ST_BLADE_ATTACK1, L"Data\\Sound\\w37\\blade_attack-01.wav"},
        {SOUND_KANTURU_1ST_BLADE_ATTACK2, L"Data\\Sound\\w37\\blade_attack-02.wav"},
        {SOUND_KANTURU_1ST_BLADE_DIE, L"Data\\Sound\\w37\\blade_death.wav"},
        {SOUND_KANTURU_1ST_SATI_MOVE1, L"Data\\Sound\\w37\\sati_idle-01.wav"},
        {SOUND_KANTURU_1ST_SATI_MOVE2, L"Data\\Sound\\w37\\sati_idle-02.wav"},
        {SOUND_KANTURU_1ST_SATI_ATTACK1, L"Data\\Sound\\w37\\sati_attack-01.wav"},
        {SOUND_KANTURU_1ST_SATI_ATTACK2, L"Data\\Sound\\w37\\sati_attack-02.wav"},
        {SOUND_KANTURU_1ST_SATI_DIE, L"Data\\Sound\\w37\\sati_death.wav"},
        {SOUND_KANTURU_1ST_SWOLF_MOVE1, L"Data\\Sound\\w37\\swolf_idle-01.wav"},
        {SOUND_KANTURU_1ST_SWOLF_MOVE2, L"Data\\Sound\\w37\\swolf_idle-02.wav"},
        {SOUND_KANTURU_1ST_SWOLF_ATTACK1, L"Data\\Sound\\w37\\swolf_attack-01.wav"},
        {SOUND_KANTURU_1ST_SWOLF_ATTACK2, L"Data\\Sound\\w37\\swolf_attack-02.wav"},
        {SOUND_KANTURU_1ST_SWOLF_DIE, L"Data\\Sound\\w37\\swolf_death.wav"},
        {SOUND_KANTURU_1ST_IR_MOVE1, L"Data\\Sound\\w37\\ir_idle-01.wav"},
        {SOUND_KANTURU_1ST_IR_MOVE2, L"Data\\Sound\\w37\\ir_idle-02.wav"},
        {SOUND_KANTURU_1ST_IR_ATTACK1, L"Data\\Sound\\w37\\ir_attack-01.wav"},
        {SOUND_KANTURU_1ST_IR_ATTACK2, L"Data\\Sound\\w37\\ir_attack-02.wav"},
        {SOUND_KANTURU_1ST_IR_DIE, L"Data\\Sound\\w37\\ir_death.wav"},
        {SOUND_KANTURU_2ND_MAPSOUND_HOLE, L"Data\\Sound\\w38\\kan_relic_hole.wav"},
        {SOUND_KANTURU_2ND_PERSO_MOVE1, L"Data\\Sound\\w38\\perso_idle-01.wav"},
        {SOUND_KANTURU_2ND_PERSO_MOVE2, L"Data\\Sound\\w38\\perso_idle-02.wav"},
        {SOUND_KANTURU_2ND_PERSO_ATTACK1, L"Data\\Sound\\w38\\perso_attack-01.wav"},
        {SOUND_KANTURU_2ND_PERSO_ATTACK2, L"Data\\Sound\\w38\\perso_attack-02.wav"},
        {SOUND_KANTURU_2ND_PERSO_DIE, L"Data\\Sound\\w38\\perso_death.wav"},
        {SOUND_KANTURU_2ND_TWIN_MOVE1, L"Data\\Sound\\w38\\twin_idle-01.wav"},
        {SOUND_KANTURU_2ND_TWIN_MOVE2, L"Data\\Sound\\w38\\twin_idle-02.wav"},
        {SOUND_KANTURU_2ND_TWIN_ATTACK1, L"Data\\Sound\\w38\\twin_attack-01.wav"},
        {SOUND_KANTURU_2ND_TWIN_ATTACK2, L"Data\\Sound\\w38\\twin_attack-02.wav"},
        {SOUND_KANTURU_2ND_TWIN_DIE, L"Data\\Sound\\w38\\twin_death.wav"},
        {SOUND_KANTURU_2ND_DRED_MOVE1, L"Data\\Sound\\w38\\dred_idle-01.wav"},
        {SOUND_KANTURU_2ND_DRED_MOVE2, L"Data\\Sound\\w38\\dred_idle-02.wav"},
        {SOUND_KANTURU_2ND_DRED_ATTACK1, L"Data\\Sound\\w38\\dred_attack-01.wav"},
        {SOUND_KANTURU_2ND_DRED_ATTACK2, L"Data\\Sound\\w38\\dred_attack-02.wav"},
        {SOUND_KANTURU_2ND_DRED_DIE, L"Data\\Sound\\w38\\dred_death.wav"},
    };
    // The models and the sounds their drawing code played: the warriors and
    // the soccer-ball Genocider play their kin's.
    struct ModelSounds
    {
        int slot = 0;
        int first = 0;
        int count = 0;
    };
    const ModelSounds models[] = {
        {MODEL_BERSERK, SOUND_KANTURU_1ST_BER_MOVE1, 5},
        {MODEL_BERSERKER_WARRIOR, SOUND_KANTURU_1ST_BER_MOVE1, 5},
        {MODEL_GIGANTIS, SOUND_KANTURU_1ST_GIGAN_MOVE1, 4},
        {MODEL_GIGANTIS_WARRIOR, SOUND_KANTURU_1ST_GIGAN_MOVE1, 4},
        {MODEL_GENOCIDER, SOUND_KANTURU_1ST_GENO_MOVE1, 5},
        {MODEL_SOCCERBALL, SOUND_KANTURU_1ST_GENO_MOVE1, 5},
        {MODEL_KENTAUROS, SOUND_KANTURU_1ST_KENTA_MOVE1, 5},
        {MODEL_KENTAUROS_WARRIOR, SOUND_KANTURU_1ST_KENTA_MOVE1, 5},
        {MODEL_BLADE_HUNTER, SOUND_KANTURU_1ST_BLADE_MOVE1, 5},
        {MODEL_SATYROS, SOUND_KANTURU_1ST_SATI_MOVE1, 5},
        {MODEL_SPLINTER_WOLF, SOUND_KANTURU_1ST_SWOLF_MOVE1, 5},
        {MODEL_IRON_RIDER, SOUND_KANTURU_1ST_IR_MOVE1, 5},
        {MODEL_PERSONA, SOUND_KANTURU_2ND_PERSO_MOVE1, 5},
        {MODEL_TWIN_TAIL, SOUND_KANTURU_2ND_TWIN_MOVE1, 5},
        {MODEL_DREADFEAR, SOUND_KANTURU_2ND_DRED_MOVE1, 5},
        {MODEL_KANTURU2ND_ENTER_NPC, SOUND_KANTURU_2ND_MAPSOUND_HOLE, 1},
    };

    std::map<int, std::wstring> modelLoads;
    for (const int world : {WD_37KANTURU_1ST, WD_38KANTURU_2ND})
    {
        MonsterRecorder::LoadScope scope(world);
        // The maps load none of them now.
        for (const MonsterRecorder::SoundEvent& sound : scope.Run([] { gMapManager.Load(); }).sounds)
        {
            INFO("world " << world << " loads " << sound.id);
            CHECK(before.find(sound.id) == before.end());
        }
        // Each model loads the sounds its code played, as the maps did.
        for (const ModelSounds& model : models)
        {
            INFO("model " << model.slot);
            std::set<int> numbers;
            for (const Data::Monsters::MonsterSoundSet* set :
                 Data::Monsters::ModelLoader::FindSoundSetsOfSlot(model.slot))
            {
                const std::set<int> own = OwnSounds(*set);
                numbers.insert(own.begin(), own.end());
            }
            std::set<int> played;
            for (int sound = model.first; sound < model.first + model.count; ++sound)
            {
                played.insert(sound);
            }
            CHECK(numbers == played);
            const int slot = model.slot;
            for (const MonsterRecorder::SoundEvent& sound :
                 scope.Run([slot] { Data::Monsters::ModelLoader::Open(slot); }).sounds)
            {
                if (!sound.release)
                {
                    CHECK(sound.channels == 1);
                    CHECK_FALSE(sound.spatial);
                    modelLoads[sound.id] = sound.file;
                }
            }
        }
    }
    CHECK(modelLoads == before);
}
