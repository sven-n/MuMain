#include "stdafx.h"

// The sounds of a model's sound slots (monsters design, P9): a slot without
// a sound (-1) plays nothing, also the second slot of a pair, which the game
// picks at random. Before, it played the number SOUND_MONSTER - 1, and for a
// monster with SubType 9 the Bull Fighter's death sound. A model without
// slots in its entry has none; before, its slots kept 0, the Bull Fighter's
// first sound.

#include "MonsterModelRecorder.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_enum.h"
#include "Engine/Object/MonsterSlotSound.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Models/ZzzBMD.h"

#include <doctest.h>

#include <algorithm>
#include <iterator>

using Engine::Object::MonsterSlotSound::Find;
using Engine::Object::MonsterSlotSound::NoSound;
using Engine::Object::MonsterSlotSound::SlotFor;

TEST_CASE("Monster models: a sound slot names its sound from SOUND_MONSTER on [monsters][sounds]")
{
    BMD model;
    std::fill(std::begin(model.Sounds), std::end(model.Sounds), NoSound);
    model.Sounds[0] = 5;
    model.Sounds[4] = 0;
    model.Sounds[5] = 12;

    CHECK(Find(model, 0) == static_cast<ESound>(SOUND_MONSTER + 5));
    CHECK(Find(model, 4) == SOUND_MONSTER_BULL1);
    // A monster with SubType 9 plays its second set, the slots from 5 on.
    CHECK(SlotFor(0, 0) == 0);
    CHECK(SlotFor(4, 0) == 4);
    CHECK(SlotFor(0, 9) == 5);
    CHECK(SlotFor(4, 9) == 9);
    CHECK(Find(model, SlotFor(0, 9)) == static_cast<ESound>(SOUND_MONSTER + 12));
}

TEST_CASE("Monster models: the monsters with SubType 9 play the sounds their second set names, as before "
          "[monsters][sounds]")
{
    // The old code played the first set's sound five numbers further on,
    // which is each slot of the second set of the two models that Hellas
    // gives SubType 9.
    for (const EMonsterModelType type : {MONSTER_MODEL_DEATH_CENTURION, MONSTER_MODEL_SHRIKER})
    {
        INFO("model " << static_cast<int>(type));
        MonsterRecorder::LoadScope scope(0);
        scope.Run([type] { OpenMonsterModel(type); });
        const MonsterRecorder::ModelSlotState state = scope.ReadSlot(static_cast<int>(MODEL_MONSTER01) + type);
        for (int slot = 0; slot < Engine::Object::MonsterSlotSound::SecondSetSlot; ++slot)
        {
            INFO("slot " << slot);
            REQUIRE(state.sounds[slot] != NoSound);
            CHECK(state.sounds[SlotFor(slot, 9)] == state.sounds[slot] + 5);
        }
    }
}

TEST_CASE("Monster models: a sound slot without a sound plays nothing, also the second of a pair (P9) "
          "[monsters][sounds]")
{
    BMD model;
    std::fill(std::begin(model.Sounds), std::end(model.Sounds), NoSound);
    model.Sounds[0] = 5;
    model.Sounds[2] = 7;

    CHECK_FALSE(Find(model, 1));
    CHECK_FALSE(Find(model, 3));
    CHECK_FALSE(Find(model, SlotFor(1, 9)));
    CHECK_FALSE(Find(model, 4));
}

TEST_CASE("Monster models: a model without sound slots has no sound in any slot (P9) [monsters][sounds]")
{
    MonsterRecorder::LoadScope scope(0);
    // The Smith's entry loads a sound but names no slots; before, its slots
    // kept 0, the Bull Fighter's first sound.
    scope.Run([] { OpenNpc(MODEL_SMITH); });
    for (const int sound : scope.ReadSlot(MODEL_SMITH).sounds)
    {
        CHECK(sound == NoSound);
    }
}

TEST_CASE("Monster models: a monster model without a body has no sound in any slot (P1, P9) [monsters][sounds]")
{
    MonsterRecorder::LoadScope scope(0);
    scope.Run([] { OpenMonsterModel(MONSTER_MODEL_MOLT); });
    for (const int sound : scope.ReadSlot(MODEL_MOLT).sounds)
    {
        CHECK(sound == NoSound);
    }
}

TEST_CASE("Monster models: the Dragon's sound slots play the sounds it loads (P9) [monsters][sounds]")
{
    MonsterRecorder::LoadScope scope(0);
    const MonsterRecorder::LoadChanges changes = scope.Run([] { OpenMonsterModel(MONSTER_MODEL_DRAGON); });
    const MonsterRecorder::ModelSlotState dragon = scope.ReadSlot(MODEL_DRAGON_);
    // Idle 0 and 1, attack 2 and 3, die 4; before, they named numbers that
    // nothing loads.
    for (int slot = 0; slot < 5; ++slot)
    {
        INFO("slot " << slot);
        const int number = SOUND_MONSTER + dragon.sounds[slot];
        CHECK(std::any_of(changes.sounds.begin(), changes.sounds.end(), [&](const MonsterRecorder::SoundEvent& event)
                          { return !event.release && event.id == number; }));
    }
}
