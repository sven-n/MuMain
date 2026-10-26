#include "stdafx.h"

// A model whose file is missing (monsters design, P1): it has no body, and
// the game goes on. Before, a monster model without its file showed a message
// box and closed the game.

#include "MonsterModelRecorder.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"
#include "Engine/Object/ZzzOpenData.h"
#ifdef _EDITOR
#include "UI/MonsterTools/MonsterShownModel.h"
#endif

#include <doctest.h>

#include <algorithm>
#include <string>
#include <vector>

namespace
{
using Data::Monsters::MonsterModelDefinition;
using MonsterRecorder::LoadChanges;
using MonsterRecorder::LoadScope;

std::vector<MonsterModelDefinition> CopyModelTable()
{
    std::vector<MonsterModelDefinition> models;
    for (const MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
    {
        models.push_back(*model);
    }
    return models;
}

// The model table with one file changed, for the life of the scope; then the
// table as it was.
class ChangedFileScope
{
public:
    ChangedFileScope(const std::string& slot, const std::string& file) : m_models(CopyModelTable())
    {
        std::vector<MonsterModelDefinition> changed = m_models;
        const auto model = std::find_if(changed.begin(), changed.end(),
                                        [&](const MonsterModelDefinition& entry) { return entry.slot == slot; });
        REQUIRE(model != changed.end());
        model->load.file = file;
        g_MonsterModelDatabase.Build(changed);
    }

    ~ChangedFileScope()
    {
        g_MonsterModelDatabase.Build(m_models);
    }

    ChangedFileScope(const ChangedFileScope&) = delete;
    ChangedFileScope& operator=(const ChangedFileScope&) = delete;

private:
    std::vector<MonsterModelDefinition> m_models;
};
} // namespace

TEST_CASE("Monster models: a monster model whose file is missing has no body, and the game goes on (P1) "
          "[monsters][models]")
{
    LoadScope scope(0);
    // Molt's file (Data/Monster/Monster50.bmd) is not in the client.
    const LoadChanges changes = scope.Run([] { OpenMonsterModel(MONSTER_MODEL_MOLT); });

    CHECK(scope.ReadSlot(MODEL_MOLT).numMeshs == 0);
    CHECK(scope.ReadSlot(MODEL_MOLT).numActions == 0);
    // A monster model without meshes gets nothing more: no texture, no sound.
    CHECK(changes.textureErrors.empty());
    CHECK(changes.sounds.empty());
}

TEST_CASE("Monster models: an NPC model whose file is missing has no body and still loads its sounds "
          "[monsters][models]")
{
    LoadScope scope(0);
    const ChangedFileScope missing("MODEL_SMITH", "Data/NPC/NoSuchNpc.bmd");
    const LoadChanges changes = scope.Run([] { OpenNpc(MODEL_SMITH); });

    CHECK(scope.ReadSlot(MODEL_SMITH).numMeshs == 0);
    REQUIRE(changes.sounds.size() == 1);
    CHECK_FALSE(changes.sounds[0].release);
}

namespace
{
int CountLoads(const LoadChanges& changes, int id)
{
    return static_cast<int>(std::count_if(changes.sounds.begin(), changes.sounds.end(),
                                          [&](const MonsterRecorder::SoundEvent& sound)
                                          { return !sound.release && sound.id == id; }));
}
} // namespace

TEST_CASE("Monster models: a sound that another model loaded from the same file is not loaded again "
          "[monsters][models][sounds]")
{
    LoadScope scope(0);
    // The Grizzly and its captain both load the Grizzly's death sound.
    const LoadChanges grizzly = scope.Run([] { OpenMonsterModel(MONSTER_MODEL_GRIZZLY); });
    const LoadChanges captain = scope.Run([] { OpenMonsterModel(MONSTER_MODEL_CAPTAIN_GRIZZLY); });
    CHECK(CountLoads(grizzly, SOUND_ELBELAND_BEASTWOO_DEATH01) == 1);
    CHECK(CountLoads(captain, SOUND_ELBELAND_BEASTWOO_DEATH01) == 0);
    // The captain's own sounds load.
    CHECK(CountLoads(captain, SOUND_ELBELAND_BEASTWOOLEADER_ATTACK01) == 1);

    // A released sound is loaded again.
    ReleaseBuffer(SOUND_ELBELAND_BEASTWOO_DEATH01);
    Models[MODEL_GRIZZLY].Release();
    const LoadChanges again = scope.Run([] { OpenMonsterModel(MONSTER_MODEL_GRIZZLY); });
    CHECK(CountLoads(again, SOUND_ELBELAND_BEASTWOO_DEATH01) == 1);
}

TEST_CASE("Monster models: the monster tools load a sound they play as its model does, once "
          "[monsters][models][sounds][editor]")
{
    LoadScope scope(0);
    // A Play button of a model that is not open loads the sound first.
    const MonsterModelDefinition* grizzly = g_MonsterModelDatabase.Find(MODEL_GRIZZLY);
    REQUIRE(grizzly != nullptr);
    REQUIRE_FALSE(grizzly->sounds.empty());
    const Data::Monsters::MonsterModelSound sound = grizzly->sounds.front();
    const int number = Data::Monsters::ModelLoader::SoundNumber(sound);
    const LoadChanges played = scope.Run(
        [&sound]
        {
            Data::Monsters::ModelLoader::LoadSound(sound);
            Data::Monsters::ModelLoader::LoadSound(sound);
        });
    CHECK(CountLoads(played, number) == 1);
    // The model then finds it loaded from the same file.
    const LoadChanges opened = scope.Run([] { OpenMonsterModel(MONSTER_MODEL_GRIZZLY); });
    CHECK(CountLoads(opened, number) == 0);
}

#ifdef _EDITOR
TEST_CASE("Monster models: the monster tools ask once for each model whose file is missing [monsters][models][editor]")
{
    LoadScope scope(0);
    const ChangedFileScope hound("MONSTER_MODEL_HOUND", "Data/Monster/MonsterNobody1.bmd");
    const ChangedFileScope bull("MONSTER_MODEL_BULL_FIGHTER", "Data/Monster/MonsterNobody2.bmd");
    // Two windows show the two models by turns, frame after frame.
    for (int frame = 0; frame < 3; ++frame)
    {
        INFO("frame " << frame);
        MuEditor::Monsters::ShownModel::OpenWhenShown(MODEL_HOUND);
        MuEditor::Monsters::ShownModel::OpenWhenShown(MODEL_BULL_FIGHTER);
        // Only the first frame asks; the files stay missing.
        CHECK(g_MonsterDataHandler.HasModelsToReopen() == (frame == 0));
        scope.Run([] { g_MonsterDataHandler.ReopenChangedModels(); });
        CHECK_FALSE(Data::Monsters::ModelLoader::IsOpen(MODEL_HOUND));
    }
}
#endif // _EDITOR
