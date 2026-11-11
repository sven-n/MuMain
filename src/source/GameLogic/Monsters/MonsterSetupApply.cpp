#include "stdafx.h"

#include "MonsterSetupApply.h"

#include "Audio/DSPlaySound.h"
#include "Character/CSParts.h"
#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Core/Text/Utf8.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Core/Utilities/Random.h"
#include "Data/GameData/EffectData/EffectTypeSymbols.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundIds.h"
#include "Data/GameData/MonsterData/UnknownMonsterLook.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Engine/AI/GOBoid.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzOpenData.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/BoneManager.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <array>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <vector>

namespace GameLogic::Monsters::Setup
{
namespace
{
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterItem;
using Data::Monsters::MonsterSetup;
using Data::Monsters::MonsterSpawnEffect;

// The spawn effects at bones a character waits to make until its first frame
// on screen, with the number and key it was made with: a character made in
// its slot since waits for nothing.
struct WaitingEffects
{
    const CHARACTER* character = nullptr;
    int number = -1;
    int key = -1;
    std::vector<MonsterSpawnEffect> effects;
};
std::vector<WaitingEffects> g_waiting;

void Wait(const CHARACTER& character, int number, int key, const std::vector<MonsterSpawnEffect>& effects)
{
    std::erase_if(g_waiting, [&](const WaitingEffects& waiting) { return waiting.character == &character; });
    WaitingEffects waiting{&character, number, key, {}};
    for (const MonsterSpawnEffect& effect : effects)
    {
        if (effect.bone)
            waiting.effects.push_back(effect);
    }
    if (!waiting.effects.empty())
        g_waiting.push_back(std::move(waiting));
}

// An item's model, or a body's slot that is no item (D63).
int ItemModel(const MonsterItem& item)
{
    if (!item.part.empty())
        return Data::Monsters::FindMonsterBodyPartSlot(item.part).value_or(-1);
    return MODEL_ITEM + item.group * MAX_ITEM_INDEX + item.number;
}

// A Dark Horse helper brings its horse, as the players' does (Q-MN57): the
// mount's SubType is its look.
std::optional<int> HelperMountLook(const std::optional<MonsterItem>& helper)
{
    if (!helper || !Data::Monsters::IsDarkHorse(*helper))
        return std::nullopt;
    return Data::Monsters::FindMountLook(helper->mountLook).value_or(0);
}

void CreateHelperMount(OBJECT& object, const std::optional<MonsterItem>& helper)
{
    if (const std::optional<int> look = HelperMountLook(helper))
        CreateMount(MODEL_DARK_HORSE, object.Position, &object, *look);
}

// A living monster's horse as its helper has it now (D55): made, taken away
// or made again in another look; one that is right already keeps going.
void UpdateHelperMount(OBJECT& object, const std::optional<MonsterItem>& helper)
{
    OBJECT* const horse =
        std::ranges::find_if(Mounts, Mounts + MAX_MOUNTS, [&](const OBJECT& mount)
                             { return mount.Live && mount.Owner == &object && mount.Type == MODEL_DARK_HORSE; });
    const std::optional<int> look = HelperMountLook(helper);
    const bool has = horse != Mounts + MAX_MOUNTS;
    if (has && look && horse->SubType == *look)
        return;
    if (has)
        horse->Live = false;
    CreateHelperMount(object, helper);
}

// A weapon or armor part: the creators wrote its type and level, and the
// excellent flags where they have them; CreateCharacterPointer cleared all
// three before.
void Wear(PART_t& part, const std::optional<MonsterItem>& item)
{
    if (!item)
        return;
    part.Type = static_cast<short>(ItemModel(*item));
    part.Level = static_cast<BYTE>(item->level);
    part.ExcellentFlags = static_cast<BYTE>(item->excellent);
}

// Wings and a helper: CreateCharacterPointer clears only their type, so a
// level or excellent flags are written only when the item has them.
void WearTypeOnly(PART_t& part, const std::optional<MonsterItem>& item)
{
    if (!item)
        return;
    part.Type = static_cast<short>(ItemModel(*item));
    if (item->level != 0)
        part.Level = static_cast<BYTE>(item->level);
    if (item->excellent != 0)
        part.ExcellentFlags = static_cast<BYTE>(item->excellent);
}

std::optional<int> EffectType(Data::Effects::EffectKind kind, const std::string& symbol)
{
    for (const auto& known : Data::Effects::GetEffectTypeSymbols(kind))
    {
        if (known.code == symbol)
            return known.type;
    }
    return std::nullopt;
}

// The equipment's slots on the character, as the creators wore them.
void WearEquipment(CHARACTER& character, const Data::Monsters::MonsterEquipment& equipment)
{
    Wear(character.Weapon[0], equipment.weapons[0]);
    Wear(character.Weapon[1], equipment.weapons[1]);
    Wear(character.BodyPart[BODYPART_HELM], equipment.helm);
    Wear(character.BodyPart[BODYPART_ARMOR], equipment.armor);
    Wear(character.BodyPart[BODYPART_PANTS], equipment.pants);
    Wear(character.BodyPart[BODYPART_GLOVES], equipment.gloves);
    Wear(character.BodyPart[BODYPART_BOOTS], equipment.boots);
    WearTypeOnly(character.Wing, equipment.wings);
    WearTypeOnly(character.Helper, equipment.helper);
}

// Gives the setup the fields of its state variants for which `isOn` is true,
// in their order. The states are read in place: a state variant holds no
// state variants, and their own list is never copied over.
template <typename TIsOn> void MergeStates(MonsterSetup& setup, TIsOn&& isOn)
{
    for (const Data::Monsters::MonsterStateVariant& variant : setup.stateVariants)
    {
        if (!isOn(variant.state))
            continue;
        for (const Data::Monsters::MonsterSetupField& field : Data::Monsters::GetMonsterSetupFields())
        {
            if (field.key != "stateVariants" && field.isSet(variant.setup))
                field.copy(setup, variant.setup);
        }
    }
}

// Whether the model has the bone: a row names it by number, and the model a
// map variant or a missing file gives may have fewer.
bool HasBone(const BMD& model, int bone)
{
    return bone >= 0 && bone < model.NumBones;
}

// A named bone of Models.json the model lacks: left out, logged once a slot
// and name.
void LogMissingBone(int slot, const std::string& name, int bone)
{
    static std::set<std::pair<int, std::string>> logged;
    if (!logged.emplace(slot, name).second)
        return;
    MU_LOG_WARN(mu::log::Get("data"), "Model slot {} has no bone {} for its named bone {}; it is left out", slot, bone,
                name);
}

// A spawn effect: on the monster and owned by it; or, at its bone or where it
// comes in (`owned` false, Q-MN38c), in the world. A row at a bone waits for a
// frame on screen, when the bone has a pose.
void Spawn(OBJECT& object, const MonsterSpawnEffect& effect, bool owned = true)
{
    vec3_t position;
    VectorCopy(object.Position, position);
    OBJECT* owner = owned ? &object : nullptr;
    if (effect.bone)
    {
        BMD& model = Models[object.Type];
        if (object.BoneTransform == nullptr || !HasBone(model, *effect.bone))
            return;
        vec3_t relative{0.0f, 0.0f, 0.0f};
        model.TransformPosition(object.BoneTransform[*effect.bone], relative, position, true);
        owner = nullptr;
    }
    if (effect.offset)
    {
        for (int i = 0; i < 3; ++i)
            position[i] += (*effect.offset)[i];
    }
    vec3_t target;
    VectorCopy(position, target);
    if (effect.targetOffset)
    {
        for (int i = 0; i < 3; ++i)
            target[i] += (*effect.targetOffset)[i];
    }
    // Each one's own random part, drawn before it is made, as the code drew it.
    const auto scale = [&]()
    {
        return effect.scaleRandom > 0 ? effect.scale + static_cast<float>(Random::Rand() % effect.scaleRandom)
                                      : effect.scale;
    };
    vec3_t light;
    if (effect.color)
    {
        Vector((*effect.color)[0], (*effect.color)[1], (*effect.color)[2], light);
    }
    else
    {
        VectorCopy(object.Light, light);
    }
    switch (effect.kind)
    {
    case MonsterSpawnEffect::Kind::Joint:
        if (const std::optional<int> type = EffectType(Data::Effects::EffectKind::Joint, effect.type))
        {
            for (int i = 0; i < effect.count; ++i)
                CreateJoint(*type, position, target, object.Angle, effect.subType, owner, scale(),
                            static_cast<short>(effect.key), 0, 0, -1, effect.color ? effect.color->data() : nullptr);
        }
        break;
    case MonsterSpawnEffect::Kind::Effect:
        if (const std::optional<int> type = EffectType(Data::Effects::EffectKind::Effect, effect.type))
        {
            for (int i = 0; i < effect.count; ++i)
                CreateEffect(*type, position, object.Angle, light, effect.subType, owner,
                             static_cast<short>(effect.key));
        }
        break;
    case MonsterSpawnEffect::Kind::Particle:
        if (const std::optional<int> type = EffectType(Data::Effects::EffectKind::Particle, effect.type))
        {
            for (int i = 0; i < effect.count; ++i)
            {
                const float size = scale();
                CreateParticle(*type, position, object.Angle, light, effect.subType, size != 0.0f ? size : 1.0f);
            }
        }
        break;
    case MonsterSpawnEffect::Kind::Object:
        if (const std::optional<int> slot = Data::Monsters::FindMonsterModelSlot(effect.type))
        {
            vec3_t angle;
            VectorCopy(object.Angle, angle);
            if (effect.angle)
                angle[2] = *effect.angle;
            CreateObject(*slot, object.Position, angle);
        }
        break;
    case MonsterSpawnEffect::Kind::Eyes:
    {
        const auto bone = [&](size_t i) { return i < effect.bones.size() ? effect.bones[i] : -1; };
        BMD& model = Models[object.Type];
        const bool posed = object.BoneTransform != nullptr && effect.bones.size() >= 2 &&
                           std::ranges::all_of(effect.bones, [&](int each) { return HasBone(model, each); });
        if (posed)
            MoveEye(&object, &model, bone(0), bone(1), bone(2), bone(3), bone(4), bone(5));
        break;
    }
    }
}

} // namespace

std::optional<int> AppearAction(const Data::Monsters::MonsterAppear& appear, int modelSlot, bool byModel)
{
    if (appear.action)
        return Data::Monsters::FindMonsterAnimation(*appear.action);
    if (byModel && modelSlot == MODEL_PLAYER)
        return PLAYER_COME_UP;
    return std::nullopt;
}

namespace
{
// An appear or a teleport (Q-MN38c), in the order of the code it replaced. An appear
// fills what it leaves out by the model: a player-shaped monster plays
// PLAYER_COME_UP and shows at once, any other fades in.
void RunAppear(CHARACTER& character, const Data::Monsters::MonsterAppear& appear, bool byModel)
{
    OBJECT& object = character.Object;
    const bool player = object.Type == MODEL_PLAYER;
    const std::optional<int> action = AppearAction(appear, object.Type, byModel);
    if (action)
    {
        SetAction(&object, *action);
        if (!appear.blend)
            object.PriorAction = static_cast<unsigned short>(*action);
    }
    if (appear.startFrame)
    {
        object.PriorAnimationFrame = *appear.startFrame;
        object.AnimationFrame = *appear.startFrame;
    }
    if (appear.appearTime)
        character.Appear = *appear.appearTime;
    std::optional<bool> fadeIn = appear.fadeIn;
    if (!fadeIn && byModel)
        fadeIn = !player;
    if (fadeIn)
        object.Alpha = *fadeIn ? 0.0f : 1.0f;
    for (const MonsterSpawnEffect& effect : appear.effects)
        Spawn(object, effect, false);
    if (appear.sound)
    {
        if (const std::optional<int> sound = Data::Monsters::FindMonsterSoundId(*appear.sound))
            PlayBuffer(static_cast<ESound>(*sound));
    }
}

// The entry of a character's number, or nullptr.
const MonsterDefinition* EntryInData(const CHARACTER& character)
{
    return g_MonsterDatabase.Find(character.MonsterIndex);
}

// What a part shows; the rest of PART_t is the character's.
struct PartStart
{
    short type = 0;
    BYTE level = 0;
    BYTE excellent = 0;
};

// A character Create made, with the values the fields living monsters take at
// once had before its setup, and the random look it drew (-1: the entry's).
struct LiveStart
{
    CHARACTER* character = nullptr;
    int number = 0;
    int key = 0;
    int variant = -1;
    // The buffs the server gave it since (NoteServerBuff).
    std::set<int> serverBuffs;
    // What it appeared with: the setup and the model.
    MonsterSetup spawned;
    std::string model;
    std::array<PartStart, 9> parts{};
    bool notRotateOnMagicHit = false;
    float scale = 1.0f;
    int subType = 0;
    WORD level = 0;
    int blendMesh = 0;
    float blendMeshLight = 0.0f;
    float edgeScale = 0.0f;
    int hiddenMesh = 0;
    bool renderShadow = false;
    bool hideShadow = false;
    float alphaTarget = 1.0f;
    bool lightEnable = false;
    BYTE pk = 0;
    bool pcRoom = false;
    bool selected = false;
    WORD moveSpeed = 0;
    bool blood = false;
};

std::vector<LiveStart> g_live;
// Counts the spawns noted, so a reader sees a spawn into the same slot.
int g_liveSpawns = 0;

// The parts in the order LiveStart keeps them.
std::array<PART_t*, 9> PartsOf(CHARACTER& character)
{
    return {&character.Weapon[0],
            &character.Weapon[1],
            &character.BodyPart[BODYPART_HELM],
            &character.BodyPart[BODYPART_ARMOR],
            &character.BodyPart[BODYPART_PANTS],
            &character.BodyPart[BODYPART_GLOVES],
            &character.BodyPart[BODYPART_BOOTS],
            &character.Wing,
            &character.Helper};
}

void NoteLive(CHARACTER& character, int number, int key, int variant, const MonsterSetup& setup,
              const std::string& model)
{
    LiveStart start;
    start.character = &character;
    start.number = number;
    start.key = key;
    start.variant = variant;
    start.spawned = setup;
    start.model = model;
    const std::array<PART_t*, 9> parts = PartsOf(character);
    for (size_t i = 0; i < parts.size(); ++i)
        start.parts[i] = {parts[i]->Type, parts[i]->Level, parts[i]->ExcellentFlags};
    const OBJECT& object = character.Object;
    start.notRotateOnMagicHit = character.NotRotateOnMagicHit;
    start.scale = object.Scale;
    start.subType = object.SubType;
    start.level = character.Level;
    start.blendMesh = object.BlendMesh;
    start.blendMeshLight = object.BlendMeshLight;
    start.edgeScale = object.m_fEdgeScale;
    start.hiddenMesh = object.HiddenMesh;
    start.renderShadow = object.m_bRenderShadow;
    start.hideShadow = character.HideShadow;
    start.alphaTarget = object.AlphaTarget;
    start.lightEnable = object.LightEnable;
    start.pk = character.PK;
    start.pcRoom = object.m_bpcroom;
    start.selected = character.m_bIsSelected;
    start.moveSpeed = character.MoveSpeed;
    start.blood = character.Blood;
    std::erase_if(g_live, [&](const LiveStart& live) { return live.character == &character; });
    g_live.push_back(start);
    ++g_liveSpawns;
}

// The fields living monsters take at once, from the values the character had
// before its setup.
void ApplyAtOnce(CHARACTER& character, const LiveStart& start, const MonsterSetup& setup)
{
    OBJECT& object = character.Object;
    const std::array<PART_t*, 9> parts = PartsOf(character);
    for (size_t i = 0; i < parts.size(); ++i)
    {
        parts[i]->Type = start.parts[i].type;
        parts[i]->Level = start.parts[i].level;
        parts[i]->ExcellentFlags = start.parts[i].excellent;
    }
    WearEquipment(character, setup.equipment);
    UpdateHelperMount(object, setup.equipment.helper);
    character.NotRotateOnMagicHit = setup.noTurnOnHit.value_or(start.notRotateOnMagicHit);
    object.Scale = setup.scale.value_or(start.scale);
    if (setup.scaleByCharacter)
        SetCharacterScale(&character);
    // A skin drawn at spawn stays until the next one.
    if (setup.randomLook.empty())
        object.SubType = setup.subType.value_or(start.subType);
    character.Level = setup.lookLevel ? static_cast<WORD>(*setup.lookLevel) : start.level;
    object.BlendMesh = setup.glowMesh.value_or(start.blendMesh);
    object.BlendMeshLight = setup.glowLight.value_or(start.blendMeshLight);
    object.m_fEdgeScale = setup.outline.value_or(start.edgeScale);
    object.HiddenMesh = setup.hiddenMesh.value_or(start.hiddenMesh);
    object.m_bRenderShadow = setup.shadow.value_or(start.renderShadow);
    character.HideShadow = setup.hideShadow.value_or(start.hideShadow);
    object.AlphaTarget = setup.alphaTarget.value_or(start.alphaTarget);
    object.LightEnable = setup.light.value_or(start.lightEnable);
    character.PK = setup.pkColor ? static_cast<BYTE>(*setup.pkColor) : start.pk;
    object.m_bpcroom = setup.helperPlate.value_or(start.pcRoom);
    character.m_bIsSelected = setup.selectable.value_or(start.selected);
    character.MoveSpeed = setup.moveSpeed ? static_cast<WORD>(*setup.moveSpeed) : start.moveSpeed;
    character.Blood = setup.noBloodPool.value_or(start.blood);
    // A fixed facing turns it now; without one it keeps the server's turn.
    if (setup.facing)
        object.Angle[2] = *setup.facing;
}

void DressCharacter(CHARACTER& character, const MonsterSetup& setup)
{
    OBJECT& object = character.Object;
    WearEquipment(character, setup.equipment);
    if (setup.playerClass)
        character.Class = static_cast<CLASS_TYPE>(*setup.playerClass);
    if (setup.classStep)
        character.Skin = static_cast<BYTE>(*setup.classStep);
    if (setup.scale)
        object.Scale = *setup.scale;
    if (setup.scaleByCharacter)
        SetCharacterScale(&character);
    if (setup.subType)
        object.SubType = *setup.subType;
    if (setup.lookLevel)
        character.Level = static_cast<WORD>(*setup.lookLevel);
    if (setup.glowMesh)
        object.BlendMesh = *setup.glowMesh;
    if (setup.glowLight)
        object.BlendMeshLight = *setup.glowLight;
    if (setup.outline)
        object.m_fEdgeScale = *setup.outline;
    if (setup.hiddenMesh)
        object.HiddenMesh = *setup.hiddenMesh;
    if (setup.shadow)
        object.m_bRenderShadow = *setup.shadow;
    if (setup.hideShadow)
        character.HideShadow = *setup.hideShadow;
    if (setup.alphaTarget)
        object.AlphaTarget = *setup.alphaTarget;
    if (setup.light)
        object.LightEnable = *setup.light;
    if (setup.pkColor)
        character.PK = static_cast<BYTE>(*setup.pkColor);
    if (setup.helperPlate)
        object.m_bpcroom = *setup.helperPlate;
}

void ApplySetup(CHARACTER& character, const MonsterSetup& setup)
{
    OBJECT& object = character.Object;
    DressCharacter(character, setup);
    if (setup.noTurnOnHit)
        character.NotRotateOnMagicHit = *setup.noTurnOnHit;
    // A random skin over the skin, as the old code drew it.
    if (!setup.randomLook.empty())
        object.SubType = setup.randomLook[static_cast<size_t>(Random::Rand()) % setup.randomLook.size()];
    if (setup.selectable)
        character.m_bIsSelected = *setup.selectable;
    if (setup.moveSpeed)
        character.MoveSpeed = static_cast<WORD>(*setup.moveSpeed);
    if (setup.noBloodPool)
        character.Blood = *setup.noBloodPool;
    if (setup.deathEffect)
        object.LifeTime = *setup.deathEffect;
    if (setup.positionOffset)
    {
        object.Position[0] += (*setup.positionOffset)[0];
        object.Position[1] += (*setup.positionOffset)[1];
    }
    if (setup.keepGroundHeight.value_or(false))
        object.Velocity = object.Position[2];
    if (setup.heightOffset)
        object.Position[2] = RequestTerrainHeight(object.Position[0], object.Position[1]) + *setup.heightOffset;
    if (setup.height)
        object.Position[2] = *setup.height;
    // Written as the creators wrote it, for an action the model has.
    if (setup.startAction && *setup.startAction >= 0 && *setup.startAction < Models[object.Type].NumActions)
        object.CurrentAction = static_cast<unsigned short>(*setup.startAction);
    if (setup.boxMin)
        std::copy(setup.boxMin->begin(), setup.boxMin->end(), object.BoundingBoxMin);
    if (setup.boxMax)
        std::copy(setup.boxMax->begin(), setup.boxMax->end(), object.BoundingBoxMax);
    if (setup.startFrame)
    {
        object.PriorAnimationFrame = *setup.startFrame;
        object.AnimationFrame = *setup.startFrame;
    }
    if (setup.randomStart)
    {
        object.Gravity = 0.0f;
        object.Distance = static_cast<float>(Random::Rand() % 20) / 10.0f;
    }
    for (const std::string& buff : setup.spawnBuffs)
    {
        if (const std::optional<int> type = Data::Monsters::FindMonsterBuff(buff))
            g_CharacterRegisterBuff((&object), static_cast<eBuffState>(*type));
    }
    for (const MonsterSpawnEffect& effect : setup.spawnEffects)
    {
        if (!effect.bone)
            Spawn(object, effect);
    }
    CreateHelperMount(object, setup.equipment.helper);
    if (setup.spawnSound)
    {
        if (const std::optional<int> sound = Data::Monsters::FindMonsterSoundId(*setup.spawnSound))
            PlayBuffer(static_cast<ESound>(*sound));
    }
    if (setup.live)
        object.Live = *setup.live;
}

bool IsObjectRow(const MonsterSpawnEffect& effect)
{
    return effect.kind == MonsterSpawnEffect::Kind::Object;
}
} // namespace

const MonsterSetup& SetupWithField(const MonsterDefinition& entry, bool (*isSet)(const MonsterSetup& setup),
                                   const SpawnWorld& world)
{
    // As SetupNow takes a field: the map variant's over the entry's, and a
    // state variant's that is on (of the map variant's list, if it has one)
    // over both, the last one winning.
    const Data::Monsters::MonsterMapVariant* map =
        Data::Monsters::FindMapVariant(entry, gMapManager.GetServerMap(), gMapManager.GetServerStage());
    const MonsterSetup* from = map != nullptr && isSet(map->setup) ? &map->setup : &entry.setup;
    const std::vector<Data::Monsters::MonsterStateVariant>& states =
        map != nullptr && !map->setup.stateVariants.empty() ? map->setup.stateVariants : entry.setup.stateVariants;
    for (const Data::Monsters::MonsterStateVariant& state : states)
    {
        if (isSet(state.setup) && world.isStateOn && world.isStateOn(state.state))
            from = &state.setup;
    }
    return *from;
}

MonsterSetup SetupNow(const MonsterDefinition& entry, const SpawnWorld& world)
{
    MonsterSetup setup = Data::Monsters::GetSetupOnMap(entry, gMapManager.GetServerMap(), gMapManager.GetServerStage());
    MergeStates(setup, [&](std::string_view state) { return world.isStateOn && world.isStateOn(state); });
    return setup;
}

void OpenModel(int slot)
{
    if (g_MonsterModelDatabase.Find(slot) == nullptr)
        return;
    if (Data::Monsters::IsMonsterRangeSlot(slot))
        OpenMonsterModel(static_cast<EMonsterModelType>(slot - MODEL_MONSTER01));
    else if (Data::Monsters::IsNpcRangeSlot(slot))
        OpenNpc(slot);
}

void Appear(CHARACTER& character, const SpawnWorld& world)
{
    const MonsterDefinition* entry = EntryInData(character);
    const Data::Monsters::MonsterAppear byModel;
    if (entry == nullptr)
    {
        RunAppear(character, byModel, true);
        return;
    }
    const MonsterSetup& from =
        SetupWithField(*entry, [](const MonsterSetup& setup) { return setup.appear.has_value(); }, world);
    RunAppear(character, from.appear ? *from.appear : byModel, true);
}

const Data::Monsters::MonsterGate* GateOf(const CHARACTER& character, const SpawnWorld& world)
{
    const MonsterDefinition* entry = EntryInData(character);
    if (entry == nullptr)
        return nullptr;
    // The gate field alone: the castle asks at every animation end and
    // packet, so no whole setup is made.
    const MonsterSetup& from =
        SetupWithField(*entry, [](const MonsterSetup& setup) { return setup.gate.has_value(); }, world);
    return from.gate ? &*from.gate : nullptr;
}

void Teleport(CHARACTER& character, const SpawnWorld& world)
{
    const MonsterDefinition* entry = EntryInData(character);
    if (entry == nullptr)
        return;
    const MonsterSetup& from =
        SetupWithField(*entry, [](const MonsterSetup& setup) { return setup.teleport.has_value(); }, world);
    if (from.teleport)
        RunAppear(character, *from.teleport, false);
}

namespace
{
// Whether the noted character still lives in its slot as Create made it,
// and is not a player's.
bool IsLive(const LiveStart& start, int number)
{
    const CHARACTER& character = *start.character;
    return (number < 0 || start.number == number) && character.Object.Live && character.MonsterIndex == start.number &&
           character.Key == start.key && character.Object.Kind != KIND_PLAYER && !character.Change;
}

// The setup the character's entry gives it now, with the equipment of the
// random look it drew.
MonsterSetup SetupOf(const MonsterDefinition& entry, const LiveStart& start, const SpawnWorld& world)
{
    MonsterSetup setup = SetupNow(entry, world);
    if (start.variant >= 0 && start.variant < static_cast<int>(entry.randomVariants.size()) &&
        entry.randomVariants[static_cast<size_t>(start.variant)].equipment)
        setup.equipment = *entry.randomVariants[static_cast<size_t>(start.variant)].equipment;
    return setup;
}

std::string ModelOf(const MonsterDefinition& entry, const LiveStart& start)
{
    if (start.variant >= 0 && start.variant < static_cast<int>(entry.randomVariants.size()))
        return entry.randomVariants[static_cast<size_t>(start.variant)].model;
    return Data::Monsters::GetModelOnMap(entry, gMapManager.GetServerMap(), gMapManager.GetServerStage());
}
} // namespace

MonsterSetup SetupFor(const MonsterDefinition& entry, const std::optional<Data::Maps::MapRef>& map,
                      std::string_view state)
{
    MonsterSetup setup = map ? Data::Monsters::GetSetupOnMap(entry, map->map, map->stage) : entry.setup;
    if (!state.empty())
        MergeStates(setup, [&](std::string_view each) { return each == state; });
    return setup;
}

void Dress(CHARACTER& character, const MonsterSetup& setup)
{
    DressCharacter(character, setup);
}

void ApplyLive(int number, const SpawnWorld& world)
{
    for (const LiveStart& start : g_live)
    {
        if (!IsLive(start, number))
            continue;
        if (const MonsterDefinition* entry = g_MonsterDatabase.Find(start.number))
            ApplyAtOnce(*start.character, start, SetupOf(*entry, start, world));
    }
}

int GetLiveSpawnCount()
{
    return g_liveSpawns;
}

std::vector<CHARACTER*> FindLive(int number)
{
    std::vector<CHARACTER*> characters;
    for (const LiveStart& start : g_live)
    {
        if (IsLive(start, number))
            characters.push_back(start.character);
    }
    return characters;
}

void CountWaiting(int number, const SpawnWorld& world, WaitingCounts& waiting)
{
    for (const LiveStart& start : g_live)
    {
        const MonsterDefinition* entry = IsLive(start, number) ? g_MonsterDatabase.Find(start.number) : nullptr;
        if (entry == nullptr)
            continue;
        if (ModelOf(*entry, start) != start.model)
            ++waiting["model"];
        const MonsterSetup now = SetupOf(*entry, start, world);
        for (const Data::Monsters::MonsterSetupField& field : Data::Monsters::GetMonsterSetupFields())
        {
            if (!field.isSame(now, start.spawned))
                ++waiting[std::string(field.key)];
        }
    }
}

void RemoveSpawnAttachments(CHARACTER& character)
{
    OBJECT& object = character.Object;
    const auto found = std::ranges::find(g_live, &character, &LiveStart::character);
    if (found != g_live.end())
    {
        for (const MonsterSpawnEffect& effect : found->spawned.spawnEffects)
        {
            if (effect.kind == MonsterSpawnEffect::Kind::Joint)
            {
                if (const std::optional<int> type = EffectType(Data::Effects::EffectKind::Joint, effect.type))
                    DeleteJoint(*type, &object, effect.subType);
            }
            else if (effect.kind == MonsterSpawnEffect::Kind::Effect)
            {
                if (const std::optional<int> type = EffectType(Data::Effects::EffectKind::Effect, effect.type))
                    DeleteEffect(*type, &object, effect.subType);
            }
        }
    }
    giPetManager::DeletePet(&character);
    DeleteCloth(&character, &object);
    DeleteParts(&character);
}

void NoteServerBuff(const OBJECT& object, int buff, bool given)
{
    // Players (the hero among them) are no living monsters: no list search
    // for each of their buffs.
    if (object.Kind == KIND_PLAYER || g_live.empty())
        return;
    const auto found =
        std::ranges::find_if(g_live, [&](const LiveStart& live) { return &live.character->Object == &object; });
    if (found == g_live.end())
        return;
    if (given)
        found->serverBuffs.insert(buff);
    else
        found->serverBuffs.erase(buff);
}

ServerState KeepServerState(const CHARACTER& character)
{
    ServerState state;
    // Every buff it shows stays except its last spawn's setup buffs that the
    // server did not give too, which go: many server paths register buffs by
    // the macro (the gate switch's open gate, the Crywolf altars' states), so
    // only the setup's own are taken off.
    const auto found = std::ranges::find(g_live, &character, &LiveStart::character);
    std::set<int> setupOnly;
    if (found != g_live.end())
    {
        for (const std::string& name : found->spawned.spawnBuffs)
        {
            const std::optional<int> buff = Data::Monsters::FindMonsterBuff(name);
            if (buff && !found->serverBuffs.contains(*buff))
                setupOnly.insert(*buff);
        }
    }
    for (const auto& [buff, count] : character.Object.m_BuffMap.m_Buff)
    {
        const int kept = static_cast<int>(buff);
        if (setupOnly.contains(kept))
            continue;
        state.buffs.push_back(kept);
        // A note of a buff it no longer shows is stale: it ends here.
        if (found != g_live.end() && found->serverBuffs.contains(kept))
            state.serverBuffs.push_back(kept);
    }
    state.buildTime = character.Object.m_byBuildTime;
    state.health = character.HealthStatus;
    state.shield = character.ShieldStatus;
    return state;
}

void RestoreServerState(CHARACTER& character, const ServerState& state)
{
    // Beside the new spawn's own buffs; those the server gave still noted.
    for (const int buff : state.buffs)
        g_CharacterRegisterBuff((&character.Object), static_cast<eBuffState>(buff));
    for (const int buff : state.serverBuffs)
        NoteServerBuff(character.Object, buff, true);
    character.Object.m_byBuildTime = state.buildTime;
    character.HealthStatus = state.health;
    character.ShieldStatus = state.shield;
}

void ReplaySpawnEffects(CHARACTER& character, const MonsterSetup& setup)
{
    for (const MonsterSpawnEffect& effect : setup.spawnEffects)
    {
        if (!IsObjectRow(effect))
            Spawn(character.Object, effect);
    }
}

void ReplayAppear(CHARACTER& character, const MonsterSetup& setup, bool sound)
{
    Data::Monsters::MonsterAppear appear = setup.appear.value_or(Data::Monsters::MonsterAppear{});
    std::erase_if(appear.effects, IsObjectRow);
    if (!sound)
        appear.sound.reset();
    RunAppear(character, appear, true);
}

void RemoveEffectsOf(const OBJECT& object)
{
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        if (Joints[i].Live && Joints[i].Target == &object)
            Joints[i].Live = false;
    }
    for (int i = 0; i < MAX_EFFECTS; ++i)
    {
        if (Effects[i].Live && Effects[i].Owner == &object)
            Effects[i].Live = false;
    }
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        if (Particles[i].Live && Particles[i].Target == &object)
            Particles[i].Live = false;
    }
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        if (Sprites[i].Live && Sprites[i].Owner == &object)
            Sprites[i].Live = false;
    }
}

void ForgetLive(const CHARACTER& character)
{
    std::erase_if(g_live, [&](const LiveStart& live) { return live.character == &character; });
    std::erase_if(g_waiting, [&](const WaitingEffects& waiting) { return waiting.character == &character; });
}

void RunWaitingSpawnEffects(CHARACTER& character)
{
    if (g_waiting.empty())
        return;
    const auto found = std::ranges::find(g_waiting, &character, &WaitingEffects::character);
    if (found == g_waiting.end())
        return;
    const WaitingEffects waiting = std::move(*found);
    g_waiting.erase(found);
    if (waiting.number != character.MonsterIndex || waiting.key != character.Key)
        return;
    for (const MonsterSpawnEffect& effect : waiting.effects)
        Spawn(character.Object, effect);
}

CHARACTER* Create(const MonsterDefinition& entry, int positionX, int positionY, int key, const SpawnWorld& world,
                  Data::Monsters::UnknownReason* why)
{
    using Data::Monsters::UnknownReason;
    // The look is drawn first, as the old code drew it: the entry's or one of
    // its random variants (Q-MN83).
    const Data::Monsters::MonsterRandomVariant* variant = nullptr;
    if (!entry.randomVariants.empty())
    {
        const int look = Random::Rand() % static_cast<int>(entry.randomVariants.size() + 1);
        if (look > 0)
            variant = &entry.randomVariants[static_cast<size_t>(look - 1)];
    }
    const int map = gMapManager.GetServerMap();
    const int stage = gMapManager.GetServerStage();
    const std::optional<int> slot = Data::Monsters::FindMonsterModelReference(
        variant ? variant->model : Data::Monsters::GetModelOnMap(entry, map, stage));
    if (!slot)
    {
        if (why != nullptr)
            *why = UnknownReason::NoModel;
        return nullptr;
    }
    OpenModel(*slot);
    // A model whose file is missing has no body: the unknown monsters' look
    // when it is for that (D64).
    if (Data::Monsters::ModelLoader::IsFileMissing(*slot) &&
        Data::Monsters::IsFor(g_MonsterDatabase.GetUnknownLook(), UnknownReason::MissingFile))
    {
        if (why != nullptr)
            *why = UnknownReason::MissingFile;
        return nullptr;
    }
    // The setup of the map and of the states that are on, with the look's
    // equipment.
    MonsterSetup setup = SetupNow(entry, world);
    if (variant && variant->equipment)
        setup.equipment = *variant->equipment;
    const std::array<int, 2> offset = setup.tileOffset.value_or(std::array<int, 2>{0, 0});
    // The body stays on the map: a tile offset past its edge stops there.
    const auto bodyTile = [](int tile, int by)
    { return static_cast<unsigned char>(std::clamp(tile + by, 0, TERRAIN_SIZE - 1)); };
    CHARACTER* character = CreateCharacter(key, *slot, bodyTile(positionX, offset[0]), bodyTile(positionY, offset[1]));
    // The body stands at the offset; the tile is the server's, on every path
    // (the packets, the cutscenes, Respawn), as the server's moves are.
    character->PositionX = static_cast<unsigned char>(positionX);
    character->PositionY = static_cast<unsigned char>(positionY);
    character->TargetX = static_cast<unsigned char>(positionX);
    character->TargetY = static_cast<unsigned char>(positionY);
    NoteLive(*character, entry.number, key,
             variant != nullptr ? static_cast<int>(variant - entry.randomVariants.data()) : -1, setup,
             variant ? variant->model : Data::Monsters::GetModelOnMap(entry, map, stage));
    // A respawn leaves the objects of its first spawn where they are and
    // runs no hooks again.
    const bool respawn = world.isRespawn && world.isRespawn();
    if (respawn)
        std::erase_if(setup.spawnEffects, IsObjectRow);
    ApplySetup(*character, setup);
    Wait(*character, entry.number, key, setup.spawnEffects);
    // A model whose file is missing has no bones: that is logged for the
    // file, not for each named bone.
    const Data::Monsters::MonsterModelDefinition* model =
        Data::Monsters::ModelLoader::IsFileMissing(*slot) ? nullptr : g_MonsterModelDatabase.Find(*slot);
    if (model != nullptr)
    {
        for (const auto& [name, bone] : model->namedBones)
        {
            // A bone the model lacks would be read outside its pose.
            if (HasBone(Models[*slot], bone))
                BoneManager::RegisterBone(character, Core::Text::FromUtf8(name), bone);
            else
                LogMissingBone(*slot, name, bone);
        }
    }
    for (const Data::Monsters::MonsterHook& hook : setup.hooks)
    {
        if (world.runHook && !respawn)
            world.runHook(hook, *character);
    }
    return character;
}
} // namespace GameLogic::Monsters::Setup
