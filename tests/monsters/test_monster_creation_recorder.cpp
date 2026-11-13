#include "stdafx.h"

// The monster recorder's creation part (monsters design, section 10): it
// records the same twice, sees the fields a creation writes and those it
// leaves, and records every monster of the list in each kind of slot.

#include "MonsterCreationRecorder.h"

#include "doctest.h"

#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Models/BoneManager.h"
#include "Render/Models/ZzzBMD.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

namespace
{
using MonsterRecorder::CreationPath;
using MonsterRecorder::CreationSlot;
using MonsterRecorder::Record;

constexpr CreationSlot Slots[] = {CreationSlot::Clean, CreationSlot::Dirty, CreationSlot::SameKey};

std::optional<std::string> Find(const Record& record, const std::string& path)
{
    const auto found = std::ranges::find(record, path, &MonsterRecorder::RecordedValue::path);
    return found != record.end() ? std::optional<std::string>(found->value) : std::nullopt;
}

std::vector<int> ListedNumbers()
{
    std::vector<int> numbers;
    for (const Data::Monsters::MonsterDefinition& monster :
         Data::Monsters::LoadMonstersFile(MonsterRecorder::ClientFolder() / "Data" / "Monsters" / "Monsters.json")
             .monsters)
    {
        numbers.push_back(monster.number);
    }
    return numbers;
}
} // namespace

TEST_CASE("Monster recorder: a creation records the same twice, in each kind of slot [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    // Budge Dragon, the Chaos Castle soldier with its random look, and Gaion
    // with his named bones.
    for (const int number : {2, 163, 504})
    {
        for (const CreationSlot slot : Slots)
        {
            INFO("monster " << number << " in a " << std::string(MonsterRecorder::Describe(slot)));
            const Record first = MonsterRecorder::RecordCreation(scope, number, slot);
            const Record second = MonsterRecorder::RecordCreation(scope, number, slot);
            const std::vector<std::string> differences = MonsterRecorder::Compare(first, second);
            std::string text;
            for (const std::string& difference : differences)
                text += difference + "\n";
            INFO(text);
            CHECK(differences.empty());
            CHECK(first.size() == second.size());
        }
    }
}

TEST_CASE("Monster recorder: the record holds what the creation writes [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const Record budge = MonsterRecorder::RecordCreation(scope, 2, CreationSlot::Clean);
    INFO(MonsterRecorder::ToText(budge));
    CHECK(Find(budge, "created") == "monster");
    CHECK(Find(budge, "object.Live") == "true");
    CHECK(Find(budge, "object.Type") == std::to_string(MODEL_BUDGE_DRAGON));
    CHECK(Find(budge, "character.MonsterIndex") == "2");
    CHECK(Find(budge, "character.Key") == std::to_string(MonsterRecorder::CreationScope::TestMonsterKey));
    CHECK(Find(budge, "object.BoneTransform") == "set");

    // Another monster writes other values.
    const Record giant = MonsterRecorder::RecordCreation(scope, 7, CreationSlot::Clean);
    CHECK(Find(giant, "object.Scale") != Find(budge, "object.Scale"));
    CHECK(Find(giant, "character.MonsterIndex") == "7");
}

TEST_CASE("Monster recorder: the record holds the joints and named bones of a creation [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    // The Bloody Wolf's two eye trails.
    const Record wolf = MonsterRecorder::RecordCreation(scope, 60, CreationSlot::Clean);
    INFO(MonsterRecorder::ToText(wolf));
    CHECK(Find(wolf, "Joints[0].Live") == "true");
    CHECK(Find(wolf, "Joints[0].Target") == "monster");
    CHECK(Find(wolf, "Joints[0].SubType") == "2");
    CHECK(Find(wolf, "Joints[1].SubType") == "3");

    // The Lunar Rabbit's bones; the record of the next creation has none of
    // them.
    const Record rabbit = MonsterRecorder::RecordCreation(scope, 413, CreationSlot::Clean);
    CHECK(Find(rabbit, "bone Rabbit_2") == "16");
    CHECK_FALSE(Find(rabbit, "Joints[0].Live"));
    CHECK_FALSE(Find(MonsterRecorder::RecordCreation(scope, 2, CreationSlot::Clean), "bone Rabbit_2"));
}

// Each path makes the monster through its handler: what the handler writes
// after the creator shows in the record.
TEST_CASE("Monster recorder: each path's handler changes the monster after its creator [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const auto record = [&](CreationPath path)
    { return MonsterRecorder::RecordCreation(scope, 2, CreationSlot::Clean, 7u, path); };
    const Record plain = record(CreationPath::CreateMonster);
    CHECK(Find(plain, "created") == "monster");

    // 0x13 turns it by the server's direction; a new one fades in.
    const Record viewport = record(CreationPath::Viewport);
    CHECK(Find(viewport, "created") == "monster");
    CHECK(Find(viewport, "object.Angle[2]") == "90");
    const Record viewportNew = record(CreationPath::ViewportNew);
    CHECK(Find(viewportNew, "object.Alpha") != Find(viewport, "object.Alpha"));
    // A teleported one keeps the flag in its key.
    CHECK(Find(record(CreationPath::ViewportTeleport), "character.Key") ==
          std::to_string(MonsterRecorder::CreationScope::KeyOf(CreationPath::ViewportTeleport)));

    // A summon is a player's; a new one comes in a magic circle.
    const Record summon = record(CreationPath::Summon);
    CHECK(Find(summon, "object.Kind") == std::to_string(KIND_PLAYER));
    CHECK_FALSE(Find(summon, "Effects[0].Type"));
    CHECK(Find(record(CreationPath::SummonNew), "Effects[0].Type") == std::to_string(MODEL_MAGIC_CIRCLE1));

    // A transformed player is a player with its class.
    const Record transform = record(CreationPath::Transform);
    CHECK(Find(transform, "object.Kind") == std::to_string(KIND_PLAYER));
    CHECK(Find(record(CreationPath::TransformNew), "Effects[0].Type") == std::to_string(MODEL_MAGIC_CIRCLE1));

    // The siege preview deletes it after a while.
    CHECK(Find(record(CreationPath::PreviewPort), "character.m_iDeleteTime") == "150");

    // A cutscene's monster has no weapons and its key.
    const Record cinematic = record(CreationPath::Cinematic);
    CHECK(Find(cinematic, "created") == "monster");
    CHECK(Find(cinematic, "character.Key") ==
          std::to_string(MonsterRecorder::CreationScope::KeyOf(CreationPath::Cinematic)));
    CHECK(Find(cinematic, "character.Weapon[0].Type") == "-1");
    INFO(MonsterRecorder::ToText(cinematic));
    CHECK(Find(cinematic, "Joints[0].Type"));
}

TEST_CASE("Monster recorder: a dirty slot shows the fields a creation leaves as they were [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const Record clean = MonsterRecorder::RecordCreation(scope, 2, CreationSlot::Clean);
    const Record dirty = MonsterRecorder::RecordCreation(scope, 2, CreationSlot::Dirty);
    // The same values written, from other values before: the records differ
    // where the dirty slot's old value was the clean one's new value, but the
    // fields the creation writes are written in both.
    CHECK(Find(dirty, "object.Type") == Find(clean, "object.Type"));
    CHECK(Find(dirty, "character.MonsterIndex") == Find(clean, "character.MonsterIndex"));
    CHECK(MonsterRecorder::Compare(clean, dirty).size() > 0);
}

// The Splinter Wolf and the Dark Ghost put their eyes on bones at spawn,
// before an animation filled them: what a freed block of the same size held
// must not show.
TEST_CASE("Monster creation: a new character's bones start at zero [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    for (int i = 0; i < 8; ++i)
    {
        auto* const used = new vec34_t[MAX_BONES];
        for (int bone = 0; bone < MAX_BONES; ++bone)
        {
            for (auto& row : used[bone])
                std::fill(std::begin(row), std::end(row), 7.0f);
        }
        delete[] used;
        CHARACTER* character = scope.Create(2);
        REQUIRE(character != nullptr);
        REQUIRE(character->Object.BoneTransform != nullptr);
        bool zero = true;
        for (int bone = 0; bone < MAX_BONES; ++bone)
        {
            for (const auto& row : character->Object.BoneTransform[bone])
                zero = zero && std::ranges::all_of(row, [](float value) { return value == 0.0f; });
        }
        CHECK(zero);
    }
}

// P14 (Q-MN44): a spawn starts from a fresh slot's values of the fields the
// slot's last character left, in a free slot and in the slot of a living
// character with the key, which also drops its named bones.
TEST_CASE("Monster creation: a spawn does not keep what the slot's last character left (P14) [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    for (const CreationSlot slot : {CreationSlot::Dirty, CreationSlot::SameKey})
    {
        INFO(std::string(MonsterRecorder::Describe(slot)));
        CHARACTER& before = MonsterRecorder::PrepareCreationSlot(slot);
        BoneManager::RegisterBone(&before, L"Stale_Bone", 3);
        CHARACTER* hound = scope.Create(MONSTER_HOUND);
        REQUIRE(hound == &before);
        const OBJECT& o = hound->Object;
        CHECK_FALSE(o.m_bpcroom);
        CHECK_FALSE(hound->HideShadow);
        CHECK(hound->Skin == 0);
        CHECK(o.LifeTime == 0.0f);
        CHECK(o.Gravity == 0.0f);
        CHECK(o.Distance == 0.0f);
        CHECK(o.m_iAnimation == 0);
        CHECK(hound->Appear == 0.0f);
        CHECK(hound->MonsterSkill == 0);
        CHECK(hound->SwordCount == 0);
        CHECK(o.PKKey == 0.0f);
        CHECK(o.EyeLeft3[0] == 0.0f);
        CHECK(o.EyeRight3[2] == 0.0f);
        CHECK(BoneManager::ListBones(hound).empty());
    }
    // Castle Gate 1 sets no link bones of its own.
    MonsterRecorder::CreationScope castle(WD_30BATTLECASTLE);
    castle.SetServerMap(WD_30BATTLECASTLE, 0);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Dirty);
    CHARACTER* gate = castle.Create(MONSTER_CASTLE_GATE1);
    REQUIRE(gate != nullptr);
    CHECK(gate->Weapon[0].LinkBone == 0);
    CHECK(gate->Weapon[1].LinkBone == 0);
    castle.SetServerMap(0, 0);
}

TEST_CASE("Monster recorder: every monster of the list is recorded in each kind of slot [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const std::vector<int> numbers = ListedNumbers();
    REQUIRE(numbers.size() > 500);
    for (const CreationSlot slot : Slots)
    {
        for (const int number : numbers)
        {
            INFO("monster " << number << " in a " << std::string(MonsterRecorder::Describe(slot)));
            const Record record = MonsterRecorder::RecordCreation(scope, number, slot);
            CHECK(Find(record, "created") == "monster");
        }
    }
}
