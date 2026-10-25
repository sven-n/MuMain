#include "stdafx.h"

#include "MonsterModelRecorder.h"

#include "Core/Globals/_enum.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Models/ZzzBMD.h"
#include "Audio/DSPlaySound.h"
#include "Audio/SoundNumbers.h"

#include <doctest.h>

#include <algorithm>
#include <string>

namespace
{
using MonsterRecorder::LoadScope;
using MonsterRecorder::Record;

const std::string* Find(const Record& record, const std::string& path)
{
    const auto value =
        std::find_if(record.begin(), record.end(), [&](const auto& recorded) { return recorded.path == path; });
    return value != record.end() ? &value->value : nullptr;
}

std::string SlotPath(int slot, const std::string& field)
{
    return "slot " + std::to_string(slot) + "." + field;
}
} // namespace

TEST_CASE("The monster recorder records what opening a monster model changes")
{
    LoadScope scope(0);
    const Record record = scope.RecordLoad([] { OpenMonsterModel(MONSTER_MODEL_HOUND); });

    const std::string* file = Find(record, SlotPath(MODEL_HOUND, "file"));
    REQUIRE(file != nullptr);
    CHECK(*file == "data\\monster\\monster02.bmd");
    CHECK(*Find(record, SlotPath(MODEL_HOUND, "action 2.speed")) == "0.34");
    CHECK(*Find(record, SlotPath(MODEL_HOUND, "action 6.hold")) == "1");
    CHECK(*Find(record, SlotPath(MODEL_HOUND, "boneHead")) == "5");
    // Textures load without a GPU and are recorded by their file.
    const std::string* bitmap = Find(record, SlotPath(MODEL_HOUND, "texture 0.bitmap"));
    REQUIRE(bitmap != nullptr);
    CHECK(bitmap->starts_with("data\\monster\\"));
    // The Hound's sounds have no SOUND_ id: they get the number of their
    // file, and the first sound slot names the first of them.
    const int hound1 = Audio::SoundNumbers::ForFile(L"Data/Sound/mHound1.wav");
    CHECK(*Find(record, "sound event 0") ==
          "load " + std::to_string(hound1) + " data\\sound\\mhound1.wav channels 2 3d");
    CHECK(*Find(record, SlotPath(MODEL_HOUND, "sound 0")) == std::to_string(hound1 - SOUND_MONSTER));
}

TEST_CASE("The monster recorder records the same load the same way in another scope")
{
    Record first;
    {
        LoadScope scope(0);
        first = scope.RecordLoad([] { OpenMonsterModel(MONSTER_MODEL_ICE_GIANT); });
    }
    LoadScope scope(0);
    const Record second = scope.RecordLoad([] { OpenMonsterModel(MONSTER_MODEL_ICE_GIANT); });
    CHECK(MonsterRecorder::Compare(first, second).empty());
    // The Ice Giant opens its six part models with it.
    CHECK(Find(first, SlotPath(MODEL_ICE_GIANT_PART6, "file")) != nullptr);
}

TEST_CASE("The monster recorder notes loads that change nothing")
{
    LoadScope scope(0);
    scope.RecordLoad([] { OpenMonsterModel(MONSTER_MODEL_HOUND); });
    // An open model is not opened again.
    CHECK(scope.RecordLoad([] { OpenMonsterModel(MONSTER_MODEL_HOUND); }).empty());
}
