#include "stdafx.h"

// A model whose file is missing (monsters design, P1): it has no body, and
// the game goes on. Before, a monster model without its file showed a message
// box and closed the game.

#include "MonsterModelRecorder.h"

#include "Core/Globals/_enum.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"
#include "Engine/Object/ZzzOpenData.h"

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
