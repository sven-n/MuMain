#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "UI/MonsterTools/MonsterAttackReaders.h"

#include <filesystem>
#include <string>
#include <vector>

using MuEditor::Monsters::AttackReaders::FindReaders;
using MuEditor::Monsters::AttackReaders::Reader;

namespace
{
const std::filesystem::path ClientData = MU_TEST_DATA_DIR;

int Slot(const char* model)
{
    const std::optional<int> slot = Data::Monsters::FindMonsterModelReference(model);
    REQUIRE(slot.has_value());
    return *slot;
}
} // namespace

TEST_CASE("Monster attack readers: the handlers answer by number, the model-keyed ones by model [editor][monsters]")
{
    CHECK(FindReaders(MONSTER_DEATH_CENTURION_1, -1) == std::vector{Reader::Hellas});
    CHECK(FindReaders(MONSTER_SELUPAN, Slot("MONSTER_MODEL_SELUPAN")) == std::vector{Reader::Raklion});
    CHECK(FindReaders(MONSTER_DEFENDER, Slot("MONSTER_MODEL_DEFENDER")) == std::vector{Reader::EmpireGuardian});
    CHECK(FindReaders(MONSTER_GAYION_THE_GLADIATOR, Slot("MONSTER_MODEL_GAYION")) ==
          std::vector{Reader::EmpireGuardian4});
    CHECK(FindReaders(MONSTER_MAYA_HAND_LEFT, Slot("MONSTER_MODEL_MAYA_HAND_LEFT")) == std::vector{Reader::Kanturu3rd});

    // The guards and the Swamp of Quiet answer for the model, whatever the number.
    CHECK(FindReaders(2, MODEL_BATTLE_GUARD1) == std::vector{Reader::BattleCastleGuards});
    CHECK(FindReaders(2, MODEL_MEDUSA) == std::vector{Reader::SwampOfQuiet});
    // A Hellas number with a Swamp model: both answer, in SetPlayerAttack's order.
    CHECK(FindReaders(MONSTER_AEGIS_1, MODEL_MEDUSA) == std::vector{Reader::Hellas, Reader::SwampOfQuiet});

    // No handler answers for the Budge Dragon.
    CHECK(FindReaders(MONSTER_BUDGE_DRAGON, Slot("MONSTER_MODEL_BUDGE_DRAGON")).empty());
}

TEST_CASE("Monster attack readers: SetPlayerAttack answers for the traps and Bali before any handler "
          "[editor][monsters]")
{
    for (const char* trap : {"mapObject 39", "mapObject 40", "mapObject 51"})
    {
        INFO(trap);
        CHECK(FindReaders(MONSTER_DEATH_CENTURION_1, Slot(trap)) == std::vector{Reader::Traps});
    }
    CHECK(FindReaders(MONSTER_BALI, Slot("MONSTER_MODEL_BALI")) == std::vector{Reader::Bali});
    CHECK(FindReaders(MONSTER_SELUPAN, Slot("MONSTER_MODEL_BALI")) == std::vector{Reader::Bali});
}

// The converter gave attacks to the monsters the handlers answered for: each
// still has a reader with its number and model.
TEST_CASE("Monster attack readers: every monster with attacks in the client's Monsters.json has a reader "
          "[editor][monsters]")
{
    const Data::Monsters::MonsterDataLoadResult loaded =
        Data::Monsters::LoadMonstersFile(ClientData / "Monsters" / "Monsters.json");
    REQUIRE(loaded.monsters.size() > 0);

    int withAttacks = 0;
    for (const Data::Monsters::MonsterDefinition& monster : loaded.monsters)
    {
        if (!monster.attacks)
        {
            continue;
        }
        ++withAttacks;
        INFO("monster " << monster.number << " " << monster.model);
        const int slot = Data::Monsters::FindMonsterModelReference(monster.model).value_or(-1);
        CHECK(slot != MODEL_PLAYER);
        CHECK_FALSE(FindReaders(monster.number, slot).empty());
    }
    CHECK(withAttacks == 115);
}

#endif // _EDITOR
