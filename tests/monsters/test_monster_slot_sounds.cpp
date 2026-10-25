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

TEST_CASE("Monster models: a sound slot names its sound from SOUND_MONSTER on [monsters][sounds]")
{
    BMD model;
    std::fill(std::begin(model.Sounds), std::end(model.Sounds), NoSound);
    model.Sounds[0] = 5;
    model.Sounds[4] = 0;

    CHECK(Find(model, 0, 0) == static_cast<ESound>(SOUND_MONSTER + 5));
    CHECK(Find(model, 4, 0) == SOUND_MONSTER_BULL1);
    // A monster with SubType 9 plays its second set, five numbers further.
    CHECK(Find(model, 0, 5) == static_cast<ESound>(SOUND_MONSTER + 10));
}

TEST_CASE("Monster models: a sound slot without a sound plays nothing, also the second of a pair (P9) "
          "[monsters][sounds]")
{
    BMD model;
    std::fill(std::begin(model.Sounds), std::end(model.Sounds), NoSound);
    model.Sounds[0] = 5;
    model.Sounds[2] = 7;

    CHECK_FALSE(Find(model, 1, 0));
    CHECK_FALSE(Find(model, 3, 0));
    CHECK_FALSE(Find(model, 1, 5));
    CHECK_FALSE(Find(model, 4, 0));
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
