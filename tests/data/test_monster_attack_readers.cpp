#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "UI/MonsterTools/MonsterAttackReaders.h"

#include <string>
#include <vector>

using MuEditor::Monsters::AttackReaders::FindReaders;
using MuEditor::Monsters::AttackReaders::Reader;

namespace
{
int Slot(const char* model)
{
    const std::optional<int> slot = Data::Monsters::FindMonsterModelReference(model);
    REQUIRE(slot.has_value());
    return *slot;
}
} // namespace

TEST_CASE("Monster attack readers: the map code that runs with the attacks, by number or model (Q-MN64) "
          "[editor][monsters]")
{
    CHECK(FindReaders(MONSTER_MAYA, Slot("MONSTER_MODEL_MAYA")) == std::vector{Reader::Kanturu3rd});
    CHECK(FindReaders(MONSTER_SELUPAN, Slot("MONSTER_MODEL_SELUPAN")) == std::vector{Reader::Raklion});
    CHECK(FindReaders(MONSTER_RAYMOND, Slot("MONSTER_MODEL_RAYMOND")) == std::vector{Reader::EmpireGuardian});
    CHECK(FindReaders(MONSTER_HAMMERIZE, Slot("MONSTER_MODEL_HAMMERIZE")) == std::vector{Reader::EmpireGuardian2});
    CHECK(FindReaders(MONSTER_GAYION_THE_GLADIATOR, Slot("MONSTER_MODEL_GAYION")) ==
          std::vector{Reader::EmpireGuardian4});
    // The guards' attack state goes with the model, whatever the number.
    CHECK(FindReaders(2, MODEL_BATTLE_GUARD1) == std::vector{Reader::BattleCastleGuards});
    for (const char* trap : {"mapObject 39", "mapObject 40", "mapObject 51"})
    {
        INFO(trap);
        CHECK(FindReaders(MONSTER_DEATH_CENTURION_1, Slot(trap)) == std::vector{Reader::Traps});
    }

    // Their attacks alone: the handlers that only played them went.
    CHECK(FindReaders(MONSTER_DEATH_CENTURION_1, -1).empty());
    CHECK(FindReaders(MONSTER_MAYA_HAND_LEFT, Slot("MONSTER_MODEL_MAYA_HAND_LEFT")).empty());
    CHECK(FindReaders(MONSTER_DEFENDER, Slot("MONSTER_MODEL_DEFENDER")).empty());
    CHECK(FindReaders(MONSTER_BALI, Slot("MONSTER_MODEL_BALI")).empty());
    CHECK(FindReaders(2, MODEL_MEDUSA).empty());
}

#endif // _EDITOR
