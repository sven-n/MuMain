#include "stdafx.h"
#include "Engine/Object/ZzzObject.h"
#include "EffectRegistry.h"
#include "Behaviors/EffectBehaviors.h"
#include "Behaviors/MoveHandlers.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <initializer_list>
#include <vector>

#if defined(_MSC_VER)
#define EFFECT_REGISTRY_NOINLINE __declspec(noinline)
#else
#define EFFECT_REGISTRY_NOINLINE __attribute__((noinline))
#endif

namespace Render::Effects
{
namespace
{
void SetComponents(vec3_t target, const CreateVector& vector)
{
    if (vector.components == 0)
        return;
    for (int i = 0; i < 3; ++i)
    {
        if (vector.components & (1 << i))
            target[i] = vector.values[i];
    }
}

// One statement per component, in the form the old cases had (`x += v *
// FPS_ANIMATION_FACTOR`), so a compiler that contracts it into a fused
// multiply-add does so for both.
void AddComponents(vec3_t target, const CreateVector& vector)
{
    if (vector.components == 0)
        return;
    for (int i = 0; i < 3; ++i)
    {
        if (!(vector.components & (1 << i)))
            continue;
        if (vector.timesFrameFactor & (1 << i))
            target[i] += vector.values[i] * FPS_ANIMATION_FACTOR;
        else
            target[i] += vector.values[i];
    }
}
} // namespace

void ApplyCreateParams(OBJECT* o, const CreateParams& params, const CreateCall& call)
{
    if (params.lifeTime)
        o->LifeTime = *params.lifeTime;
    if (params.scale)
        o->Scale = *params.scale;
    if (params.velocity)
        o->Velocity = *params.velocity;
    if (params.gravity)
        o->Gravity = *params.gravity;
    if (params.hiddenMesh)
        o->HiddenMesh = *params.hiddenMesh;
    if (params.blendMesh)
        o->BlendMesh = *params.blendMesh;
    if (params.blendMeshLight)
        o->BlendMeshLight = *params.blendMeshLight;
    if (params.alpha)
        o->Alpha = *params.alpha;
    if (params.light)
        VectorCopy(params.light->data(), o->Light);
    if (params.groups == 0)
        return;

    if (params.groups & CreateParams::Flags)
    {
        if (params.lightEnable)
            o->LightEnable = *params.lightEnable;
        if (params.alphaEnable)
            o->AlphaEnable = *params.alphaEnable;
        if (params.kind)
            o->Kind = *params.kind;
        if (params.skill)
            o->Skill = *params.skill;
    }
    if (params.groups & CreateParams::Numbers)
    {
        if (params.pkKey)
            o->PKKey = *params.pkKey;
        if (params.timer)
            o->Timer = *params.timer;
        if (params.distance)
            o->Distance = *params.distance;
        if (params.collisionRange)
            o->CollisionRange = *params.collisionRange;
    }
    if (params.groups & CreateParams::Vectors)
    {
        SetComponents(o->Position, params.position);
        SetComponents(o->Angle, params.angle);
        SetComponents(o->Direction, params.direction);
    }
    if (params.groups & CreateParams::Offsets)
    {
        AddComponents(o->Position, params.positionOffset);
        AddComponents(o->Angle, params.angleOffset);
    }
    if (params.groups & CreateParams::Copies)
    {
        if (params.copyLightToDirection)
            VectorCopy(o->Light, o->Direction);
        if (params.copyPositionToStartPosition)
            VectorCopy(o->Position, o->StartPosition);
        if (params.copyCallLightToHeadTargetAngle)
            VectorCopy(call.light, o->HeadTargetAngle);
        if (params.copyCallScaleToScale)
            o->Scale = call.scale;
    }
    // After the copy into it.
    if (params.groups & CreateParams::Offsets)
        AddComponents(o->StartPosition, params.startPositionOffset);
}

namespace
{
std::optional<float> ToFloat(const std::optional<double>& value)
{
    if (!value)
        return std::nullopt;
    return static_cast<float>(*value);
}

CreateVector ToCreateVector(const Data::Effects::EffectCreateVector& vector)
{
    CreateVector converted;
    for (int i = 0; i < 3; ++i)
    {
        if (!vector.components[i])
            continue;
        converted.values[i] = static_cast<float>(*vector.components[i]);
        converted.components |= static_cast<std::uint8_t>(1 << i);
        if (vector.timesFrameFactor[i])
            converted.timesFrameFactor |= static_cast<std::uint8_t>(1 << i);
    }
    return converted;
}

template <typename T> std::optional<T> ToInteger(const std::optional<int>& value)
{
    if (!value)
        return std::nullopt;
    return static_cast<T>(*value);
}

// The values of the catalogue as the effects use them.
CreateParams ToCreateParams(const Data::Effects::EffectCreateParams& values)
{
    CreateParams params;
    params.lifeTime = ToFloat(values.lifeTime);
    params.scale = ToFloat(values.scale);
    params.velocity = ToFloat(values.velocity);
    params.gravity = ToFloat(values.gravity);
    params.hiddenMesh = values.hiddenMesh;
    params.blendMesh = values.blendMesh;
    params.blendMeshLight = ToFloat(values.blendMeshLight);
    params.alpha = ToFloat(values.alpha);
    if (values.light)
    {
        const std::array<double, 3>& light = *values.light;
        params.light = std::array<float, 3>{static_cast<float>(light[0]), static_cast<float>(light[1]),
                                            static_cast<float>(light[2])};
    }

    params.lightEnable = values.lightEnable;
    params.alphaEnable = values.alphaEnable;
    params.kind = ToInteger<std::uint8_t>(values.kind);
    params.skill = ToInteger<std::uint16_t>(values.skill);
    params.pkKey = ToFloat(values.pkKey);
    params.timer = ToFloat(values.timer);
    params.distance = ToFloat(values.distance);
    params.collisionRange = ToFloat(values.collisionRange);
    if (params.lightEnable || params.alphaEnable || params.kind || params.skill)
        params.groups |= CreateParams::Flags;
    if (params.pkKey || params.timer || params.distance || params.collisionRange)
        params.groups |= CreateParams::Numbers;

    params.position = ToCreateVector(values.position);
    params.angle = ToCreateVector(values.angle);
    params.direction = ToCreateVector(values.direction);
    if (params.position.components || params.angle.components || params.direction.components)
        params.groups |= CreateParams::Vectors;

    params.positionOffset = ToCreateVector(values.positionOffset);
    params.angleOffset = ToCreateVector(values.angleOffset);
    params.startPositionOffset = ToCreateVector(values.startPositionOffset);
    if (params.positionOffset.components || params.angleOffset.components || params.startPositionOffset.components)
        params.groups |= CreateParams::Offsets;

    params.copyLightToDirection = values.copyLightToDirection;
    params.copyPositionToStartPosition = values.copyPositionToStartPosition;
    params.copyCallLightToHeadTargetAngle = values.copyCallLightToHeadTargetAngle;
    params.copyCallScaleToScale = values.copyCallScaleToScale;
    if (params.copyLightToDirection || params.copyPositionToStartPosition || params.copyCallLightToHeadTargetAngle ||
        params.copyCallScaleToScale)
        params.groups |= CreateParams::Copies;
    return params;
}

struct Entry
{
    int type;
    EffectDescriptor descriptor;
};

// Built by BuildTable (from BuildRegistry, or from the first lookup before
// it): the descriptors, and the table indexed by type that points into them.
std::vector<Entry> builtEntries;
std::vector<const EffectDescriptor*> table;

Entry& FindOrAdd(std::vector<Entry>& entries, int type)
{
    for (Entry& entry : entries)
    {
        if (entry.type == type)
            return entry;
    }
    return entries.emplace_back(Entry{type, EffectDescriptor{}});
}

// The handlers of the code. Each row states only what differs from
// the legacy switch statements in ZzzEffect.cpp; the creation values
// come from the effect catalogue (Data/Effects/EffectTypes.json).
std::vector<Entry> HandlerEntries()
{
    std::vector<Entry> e;
    auto add = [&e](std::initializer_list<int> types, const EffectDescriptor& d)
    {
        for (int type : types)
        {
            EffectDescriptor& descriptor = FindOrAdd(e, type).descriptor;
            if (d.onCreate)
                descriptor.onCreate = d.onCreate;
            if (d.move)
                descriptor.move = d.move;
            if (d.render)
                descriptor.render = d.render;
        }
    };

    // MODEL_DESAIR: rides a joint and sheds feathers (see
    // Behaviors::MoveDesair). Default render.
    add({MODEL_DESAIR}, {.move = &Behaviors::MoveDesair, .render = &Behaviors::RenderDefault});

    // --- Effects with creation values and a move handler ---------
    add({MODEL_MAGIC_CAPSULE2}, {.move = &Behaviors::MoveMagicCapsule2});
    add({MODEL_SPEAR}, {.move = &Behaviors::MoveSpear});
    add({MODEL_SUMMONER_SUMMON_NEIL_NIFE1, MODEL_SUMMONER_SUMMON_NEIL_NIFE2, MODEL_SUMMONER_SUMMON_NEIL_NIFE3},
        {.move = &Behaviors::MoveSummonerNeilNife});
    add({MODEL_SUMMONER_SUMMON_NEIL_GROUND1, MODEL_SUMMONER_SUMMON_NEIL_GROUND2, MODEL_SUMMONER_SUMMON_NEIL_GROUND3},
        {.move = &Behaviors::MoveSummonerNeilGround});
    add({BITMAP_FIRE_RED}, {.move = &Behaviors::MoveBitmapFireRed});
    add({BITMAP_LIGHT_MARKS}, {.move = &Behaviors::MoveBitmapLightMarks});
    add({MODEL_MAGIC1}, {.move = &Behaviors::MoveMagic1});
    add({MODEL_MAYASTAR}, {.move = &Behaviors::MoveMayaStar});
    add({BITMAP_FIRE}, {.move = &Behaviors::MoveBitmapFire});
    add({MODEL_INFINITY_ARROW4}, {.move = &Behaviors::MoveInfinityArrow4});

    // --- Randomised / directional creation via an onCreate hook ---
    add({MODEL_MAYASTONE4, MODEL_MAYASTONE5}, {.onCreate = &Behaviors::CreateMayaStone45});

    // --- Move handlers mechanically extracted from MoveEffect -----
    // (see Behaviors/MoveHandlers.cpp). Creation for these types still
    // runs through the legacy switch unless the catalogue gives them
    // creation values, and rendering unless a render handler is listed
    // above.
    for (const auto& [type, move] : Behaviors::ExtractedMoveHandlers())
        FindOrAdd(e, type).descriptor.move = move;

    return e;
}

void BuildTable(std::span<const Data::Effects::EffectTypeCreateParams> createParams)
{
    std::vector<Entry> entries = HandlerEntries();
    for (const Data::Effects::EffectTypeCreateParams& row : createParams)
        FindOrAdd(entries, row.type).descriptor.create = ToCreateParams(row.params);

    int maxType = -1;
    for (const Entry& entry : entries)
        maxType = (entry.type > maxType) ? entry.type : maxType;

    builtEntries = std::move(entries);
    table.assign(maxType + 1, nullptr);
    for (const Entry& entry : builtEntries)
        table[entry.type] = &entry.descriptor;
}

// The handlers are code, so they work without the catalogue (tests, tools);
// only its creation values are missing until BuildRegistry. Kept out of
// Lookup, so that a lookup stays a bounds check and one array read.
EFFECT_REGISTRY_NOINLINE const EffectDescriptor* LookupBeforeBuild(int type)
{
    BuildTable({});
    MU_LOG_ERROR(mu::log::Get("render"),
                 "Effect type {} was used before the effect catalogue was loaded; the effects run without the "
                 "creation values of the catalogue until it is loaded",
                 type);
    if (type < 0 || type >= static_cast<int>(table.size()))
        return nullptr;
    return table[type];
}
} // namespace

void BuildRegistry(std::span<const Data::Effects::EffectTypeCreateParams> createParams)
{
    BuildTable(createParams);
}

const EffectDescriptor* Lookup(int type)
{
    if (type < 0 || type >= static_cast<int>(table.size()))
        return table.empty() ? LookupBeforeBuild(type) : nullptr;
    return table[type];
}
} // namespace Render::Effects
