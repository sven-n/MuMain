#include "stdafx.h"

// One-time comparison of MN3 (monsters design, D50): every monster and NPC
// model opens from Data/Monsters/Models.json exactly as the model code opened
// it. Each slot is opened twice in one build: with an empty model table, so
// OpenMonsterModel and OpenNpc run their old code, and with the shipped
// table. It goes with the old code.

#include "MonsterModelRecorder.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/EffectData/EffectDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterModelJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Models/ZzzBMD.h"
#include "World/MapInfra/MapManager.h"

#include <doctest.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <functional>
#include <memory>
#include <map>
#include <string>
#include <vector>

namespace
{
using MonsterRecorder::LoadChanges;
using MonsterRecorder::LoadScope;
using MonsterRecorder::ModelSlotState;

// Molt's file is missing: its old code closes the game (P1).
constexpr int MoltType = MONSTER_MODEL_MOLT;

const std::vector<Data::Monsters::MonsterModelDefinition>& ShippedModels()
{
    static const std::vector<Data::Monsters::MonsterModelDefinition> models = []
    {
        LoadScope scope(0);
        std::string error;
        REQUIRE(Data::Effects::LoadEffectTypes(error));
        Data::Monsters::MonsterModelsLoadResult result =
            Data::Monsters::LoadMonsterModelsFile(Data::Monsters::GetMonsterModelsFile());
        REQUIRE_FALSE(Data::Items::HasErrors(result.issues));
        return result.models;
    }();
    return models;
}

bool IsLoaded(const ModelSlotState& state)
{
    return state.numMeshs > 0 || state.numActions > 0;
}

// The numbers that sounds without a SOUND_* id got, as the ids the old code
// loaded the same files with, so the two records can be compared.
void NumberSoundsAsOld(LoadChanges& data, const LoadChanges& old)
{
    std::map<int, int> oldIds;
    for (const MonsterRecorder::SoundEvent& event : data.sounds)
    {
        if (event.id < static_cast<int>(MAX_BUFFER))
            continue;
        const auto match =
            std::find_if(old.sounds.begin(), old.sounds.end(), [&](const MonsterRecorder::SoundEvent& oldEvent)
                         { return !oldEvent.release && oldEvent.file == event.file; });
        if (match != old.sounds.end())
            oldIds[event.id] = match->id;
    }
    for (MonsterRecorder::SoundEvent& event : data.sounds)
    {
        if (const auto oldId = oldIds.find(event.id); oldId != oldIds.end())
            event.id = oldId->second;
    }
    for (auto& [slot, state] : data.slots)
    {
        for (int& value : state.sounds)
        {
            if (value == -1)
                continue;
            if (const auto oldId = oldIds.find(SOUND_MONSTER + value); oldId != oldIds.end())
                value = oldId->second - SOUND_MONSTER;
        }
    }
}

// What the old code repeats changes nothing: the Dark Phoenix's shield loads
// the same sound twice in a row (a load replaces the sound), and OpenNpc opens
// the textures twice from the same folder, so a texture no folder has is
// reported twice. Each is kept once, and the errors without case (the data
// spells folders as they are on disk: "NPC" for "Npc").
void LeaveOutRepeats(LoadChanges& changes)
{
    std::vector<MonsterRecorder::SoundEvent> sounds;
    for (const MonsterRecorder::SoundEvent& event : changes.sounds)
    {
        const bool repeated = !sounds.empty() && !event.release && !sounds.back().release &&
                              sounds.back().id == event.id && sounds.back().file == event.file &&
                              sounds.back().channels == event.channels && sounds.back().spatial == event.spatial;
        if (!repeated)
            sounds.push_back(event);
    }
    changes.sounds = std::move(sounds);

    std::vector<std::string> errors;
    for (std::string error : changes.textureErrors)
    {
        std::transform(error.begin(), error.end(), error.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (std::find(errors.begin(), errors.end(), error) == errors.end())
            errors.push_back(error);
    }
    std::sort(errors.begin(), errors.end());
    changes.textureErrors = std::move(errors);
}

// The old code of the Yeti and the Elite Yeti also writes the head bone and
// sound slots of the other's slot. These writes into a slot that is not open
// are left out of the comparison; the test checks that they are what the
// other's own old code sets, which its entry sets as well.
std::vector<std::pair<int, ModelSlotState>> TakeWritesIntoClosedSlots(LoadChanges& old)
{
    std::vector<std::pair<int, ModelSlotState>> writes;
    const auto closed = std::stable_partition(old.slots.begin(), old.slots.end(),
                                              [](const auto& slot) { return IsLoaded(slot.second); });
    writes.assign(closed, old.slots.end());
    old.slots.erase(closed, old.slots.end());
    return writes;
}

// What the creators wrote into the shared models when a monster appeared
// (the values are in the model entries now),
// copied as it was (ZzzCharacter.cpp CreateMonster, GMNewTown.cpp), only
// kept within the animations and meshes the files have.
void SetSpeed(int slot, int animation, float speed)
{
    if (animation < Models[slot].NumActions)
        Models[slot].Actions[animation].PlaySpeed = speed;
}

struct CreatorWrite
{
    int slot;
    std::function<void()> write;
};

std::vector<CreatorWrite> MakeCreatorWrites()
{
    std::vector<CreatorWrite> writes = {
        {MODEL_DRAKAN,
         []
         {
             BMD& model = Models[MODEL_DRAKAN];
             const bool noneBlend[] = {true, false, false, true, true};
             for (int mesh = 0; mesh < 5 && mesh < model.NumMeshs; ++mesh)
                 model.Meshs[mesh].NoneBlendMesh = noneBlend[mesh];
         }},
        {MODEL_DARK_PHEONIX_SHIELD, [] { Models[MODEL_DARK_PHEONIX_SHIELD].StreamMesh = 0; }},
        {MODEL_GAMBLE_NPC_MOSS,
         []
         {
             for (int i = 0; i < 6; i++)
                 SetSpeed(MODEL_GAMBLE_NPC_MOSS, i, 0.33f);
         }},
        {MODEL_LUCKYITEM_NPC,
         []
         {
             SetSpeed(MODEL_LUCKYITEM_NPC, 0, 0.45f);
             SetSpeed(MODEL_LUCKYITEM_NPC, 1, 0.5f);
         }},
        {MODEL_ELBELAND_RHEA,
         []
         {
             SetSpeed(MODEL_ELBELAND_RHEA, 0, 0.2f);
             SetSpeed(MODEL_ELBELAND_RHEA, 1, 0.4f);
         }},
    };
    for (int santa = 0; santa < 8; ++santa)
    {
        const int slot = MODEL_LITTLESANTA + santa;
        writes.push_back({slot, [slot]
                          {
                              for (int i = 0; i < 5; i++)
                                  SetSpeed(slot, i, i < 2 || i == 4 ? 0.4f : 0.5f);
                          }});
    }
    return writes;
}

// The creator's writes into the slot's model, or nothing.
std::function<void()> CreatorWriteOf(int slot)
{
    static const std::vector<CreatorWrite> writes = MakeCreatorWrites();
    for (const CreatorWrite& write : writes)
    {
        if (write.slot == slot)
            return write.write;
    }
    return [] {};
}

struct Comparison
{
    std::vector<std::string> differences;
    std::vector<std::pair<int, ModelSlotState>> writesIntoClosedSlots;
};

// `creatorWrite` follows the old code: what the creator of a monster of the
// model wrote into it when the monster appeared.
Comparison Compare(int world, const std::function<void()>& open, const std::function<void()>& creatorWrite = [] {})
{
    Comparison comparison;
    LoadChanges old;
    {
        g_MonsterModelDatabase.Build({});
        LoadScope scope(world);
        old = scope.Run(
            [&]
            {
                open();
                creatorWrite();
            });
    }
    LoadChanges data;
    {
        g_MonsterModelDatabase.Build(ShippedModels());
        LoadScope scope(world);
        data = scope.Run(open);
    }
    comparison.writesIntoClosedSlots = TakeWritesIntoClosedSlots(old);
    LeaveOutRepeats(old);
    LeaveOutRepeats(data);
    NumberSoundsAsOld(data, old);
    comparison.differences = MonsterRecorder::Compare(MonsterRecorder::ToRecord(old), MonsterRecorder::ToRecord(data));
    return comparison;
}

// What a slot holds after its own old code opened it.
ModelSlotState OpenedByOldCode(int slot)
{
    g_MonsterModelDatabase.Build({});
    LoadScope scope(0);
    if (Data::Monsters::IsMonsterRangeSlot(slot))
        OpenMonsterModel(static_cast<EMonsterModelType>(slot - MODEL_MONSTER01));
    else
        OpenNpc(slot);
    return scope.ReadSlot(slot);
}

bool HasEntry(int slot)
{
    return std::any_of(ShippedModels().begin(), ShippedModels().end(),
                       [&](const auto& model) { return Data::Monsters::FindMonsterModelSlot(model.slot) == slot; });
}

void CheckMonsterModels(int world)
{
    int compared = 0;
    for (int type = 0; type < MONSTER_MODEL_COUNT; ++type)
    {
        if (type == MoltType || !HasEntry(MODEL_MONSTER01 + type))
            continue;
        const Comparison comparison = Compare(
            world, [type] { OpenMonsterModel(static_cast<EMonsterModelType>(type)); },
            CreatorWriteOf(MODEL_MONSTER01 + type));
        INFO("monster model type " << type << " (" << Data::Monsters::GetMonsterModelTypeName(type) << ") on world "
                                   << world);
        for (const std::string& difference : comparison.differences)
        {
            INFO(difference);
            CHECK(false);
        }
        for (const auto& [slot, written] : comparison.writesIntoClosedSlots)
        {
            const ModelSlotState own = OpenedByOldCode(slot);
            CHECK(written.boneHead == own.boneHead);
            CHECK(written.sounds == own.sounds);
        }
        ++compared;
    }
    CHECK(compared > 0);
}
} // namespace

TEST_CASE("Monster models: Models.json opens the monster models as the model code did [monsters][comparison]")
{
    CheckMonsterModels(0);
}

TEST_CASE("Monster models: in Chaos Castle as the model code did [monsters][comparison]")
{
    CheckMonsterModels(WD_18CHAOS_CASTLE);
}

TEST_CASE("Monster models: in Battle Castle as the model code did [monsters][comparison]")
{
    CheckMonsterModels(WD_30BATTLECASTLE);
}

TEST_CASE("Monster models: Models.json opens the NPC models as the model code did [monsters][comparison]")
{
    ShippedModels();
    for (int slot = MODEL_NPC_BEGIN + 1; slot < MODEL_NPC_END; ++slot)
    {
        const Comparison comparison = Compare(0, [slot] { OpenNpc(slot); }, CreatorWriteOf(slot));
        INFO("NPC slot " << slot << " (" << Data::Monsters::GetMonsterModelSymbol(slot) << ")");
        for (const std::string& difference : comparison.differences)
        {
            INFO(difference);
            CHECK(false);
        }
        CHECK(comparison.writesIntoClosedSlots.empty());
    }
}

TEST_CASE("Monster models: the comparison opens the models from the data [monsters][comparison]")
{
    // A sound without an id gets a number after the SOUND_* list only when
    // the entry opens the model.
    g_MonsterModelDatabase.Build(ShippedModels());
    LoadScope scope(0);
    const LoadChanges changes = scope.Run([] { OpenMonsterModel(MONSTER_MODEL_HOUND); });
    REQUIRE_FALSE(changes.sounds.empty());
    CHECK(changes.sounds.front().id >= static_cast<int>(MAX_BUFFER));
    g_MonsterModelDatabase.Build({});
}

namespace
{
// Every map's load, with the part models it opens for monsters. Raklion's
// load needs the terrain (CreateObject) and cannot run here; its code opens
// no monster part.
void CheckMapLoads(int firstWorld, int lastWorld)
{
    ShippedModels();
    for (int world = firstWorld; world <= lastWorld; ++world)
    {
        if (world == WD_57ICECITY || world == WD_58ICECITY_BOSS)
            continue;
        const Comparison comparison = Compare(world, [] { gMapManager.Load(); });
        INFO("map load of world " << world);
        for (const std::string& difference : comparison.differences)
        {
            INFO(difference);
            CHECK(false);
        }
    }
}
} // namespace

TEST_CASE("Monster models: the maps open the monster part models as their code did, worlds 0-40 [monsters][comparison]")
{
    CheckMapLoads(0, 40);
}

TEST_CASE(
    "Monster models: the maps open the monster part models as their code did, worlds 41-81 [monsters][comparison]")
{
    CheckMapLoads(41, NUM_WD - 1);
}

TEST_CASE("Monster models: the comparison opens the map parts from the data [monsters][comparison]")
{
    g_MonsterModelDatabase.Build(ShippedModels());
    REQUIRE(Data::Monsters::ModelLoader::HasMapParts());
    LoadScope scope(WD_51HOME_6TH_CHAR);
    const LoadChanges changes = scope.Run([] { Data::Monsters::ModelLoader::OpenMapParts(WD_51HOME_6TH_CHAR); });
    const auto totemPart = std::find_if(changes.slots.begin(), changes.slots.end(),
                                        [](const auto& slot) { return slot.first == MODEL_TOTEMGOLEM_PART1; });
    REQUIRE(totemPart != changes.slots.end());
    CHECK(IsLoaded(totemPart->second));
    g_MonsterModelDatabase.Build({});
}

namespace
{
struct CharacterBody
{
    std::array<float, 3> boxMin{};
    std::array<float, 3> boxMax{};
    int rightHandBone = 0;
    int leftHandBone = 0;
    int swordCount = 0;

    bool operator==(const CharacterBody&) const = default;
};

// What CreateCharacterPointer gives a character of the slot, on a character
// whose bones and attack count were set to values no model uses.
CharacterBody CreateCharacterOf(int slot)
{
    constexpr BYTE Untouched = 250;
    auto character = std::make_unique<CHARACTER>();
    character->Weapon[0].LinkBone = Untouched;
    character->Weapon[1].LinkBone = Untouched;
    character->SwordCount = 9;
    CreateCharacterPointer(character.get(), slot, 10, 10, 0.0f);
    const OBJECT& object = character->Object;
    CharacterBody body;
    body.boxMin = {object.BoundingBoxMin[0], object.BoundingBoxMin[1], object.BoundingBoxMin[2]};
    body.boxMax = {object.BoundingBoxMax[0], object.BoundingBoxMax[1], object.BoundingBoxMax[2]};
    body.rightHandBone = character->Weapon[0].LinkBone;
    body.leftHandBone = character->Weapon[1].LinkBone;
    body.swordCount = character->SwordCount;
    delete[] character->Object.BoneTransform;
    character->Object.BoneTransform = nullptr;
    return body;
}
} // namespace

TEST_CASE("Monster models: characters get the bounding box and weapon bones of Models.json as the code gave them "
          "[monsters][comparison]")
{
    ShippedModels();
    int withEntry = 0;
    for (int slot = 0; slot < MAX_MODELS; ++slot)
    {
        g_MonsterModelDatabase.Build({});
        const CharacterBody old = CreateCharacterOf(slot);
        g_MonsterModelDatabase.Build(ShippedModels());
        const CharacterBody data = CreateCharacterOf(slot);
        withEntry += g_MonsterModelDatabase.Find(slot) != nullptr ? 1 : 0;
        INFO("slot " << slot << " (" << Data::Monsters::GetMonsterModelSymbol(slot) << ")");
        CHECK(old.boxMin == data.boxMin);
        CHECK(old.boxMax == data.boxMax);
        CHECK(old.rightHandBone == data.rightHandBone);
        CHECK(old.leftHandBone == data.leftHandBone);
        CHECK(old.swordCount == data.swordCount);
    }
    CHECK(withEntry == static_cast<int>(ShippedModels().size()));
    g_MonsterModelDatabase.Build({});
}
