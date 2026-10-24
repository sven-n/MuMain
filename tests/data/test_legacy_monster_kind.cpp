#include "stdafx.h"

#include "doctest.h"

#include "Core/Globals/_define.h"
#include "Data/GameData/MonsterData/LegacyMonsterKind.h"

using Data::Monsters::GetLegacyObjectKind;

TEST_CASE("Legacy monster kinds: the number ranges of the old client [data][monsters]")
{
    CHECK(GetLegacyObjectKind(0) == KIND_MONSTER);
    CHECK(GetLegacyObjectKind(99) == KIND_MONSTER);
    CHECK(GetLegacyObjectKind(100) == KIND_TRAP);
    CHECK(GetLegacyObjectKind(110) == KIND_TRAP);
    CHECK(GetLegacyObjectKind(111) == KIND_MONSTER);
    CHECK(GetLegacyObjectKind(200) == KIND_MONSTER);
    CHECK(GetLegacyObjectKind(201) == KIND_NPC);
    CHECK(GetLegacyObjectKind(259) == KIND_NPC);
    CHECK(GetLegacyObjectKind(260) == KIND_MONSTER);
    CHECK(GetLegacyObjectKind(1023) == KIND_MONSTER);
}

TEST_CASE("Legacy monster kinds: the numbers the old client set apart [data][monsters]")
{
    CHECK(GetLegacyObjectKind(451) == KIND_TMP);
    CHECK(GetLegacyObjectKind(480) == KIND_MONSTER);
    CHECK(GetLegacyObjectKind(491) == KIND_MONSTER);
    CHECK(GetLegacyObjectKind(368) == KIND_NPC);
    CHECK(GetLegacyObjectKind(370) == KIND_NPC);
    CHECK(GetLegacyObjectKind(450) == KIND_NPC);
    CHECK(GetLegacyObjectKind(543) == KIND_NPC);
    CHECK(GetLegacyObjectKind(568) == KIND_NPC);
    // Not in the old client's list of NPCs (P7 of the monsters design).
    CHECK(GetLegacyObjectKind(566) == KIND_MONSTER);
    CHECK(GetLegacyObjectKind(567) == KIND_MONSTER);
}
