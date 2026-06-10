#include "stdafx.h"
#include "Engine/Object/ZzzObject.h"
#include "EffectRegistry.h"
#include "Behaviors/EffectBehaviors.h"

#include <initializer_list>
#include <vector>

namespace Render::Effects
{
    void ApplyCreateParams(OBJECT* o, const CreateParams& params)
    {
        if (params.lifeTime)       o->LifeTime = *params.lifeTime;
        if (params.scale)          o->Scale = *params.scale;
        if (params.velocity)       o->Velocity = *params.velocity;
        if (params.gravity)        o->Gravity = *params.gravity;
        if (params.hiddenMesh)     o->HiddenMesh = *params.hiddenMesh;
        if (params.blendMesh)      o->BlendMesh = *params.blendMesh;
        if (params.blendMeshLight) o->BlendMeshLight = *params.blendMeshLight;
        if (params.alpha)          o->Alpha = *params.alpha;
        if (params.light)          VectorCopy(params.light->data(), o->Light);
        if (params.copyLightToDirection) VectorCopy(o->Light, o->Direction);
    }

    namespace
    {
        struct Entry
        {
            int              type;
            EffectDescriptor descriptor;
        };

        // The migrated effects. Each row states only what differs from the
        // common initialisation CreateEffect applies to every effect. Effects
        // absent from this list keep being handled by the legacy switch
        // statements in ZzzEffect.cpp.
        const std::vector<Entry>& Entries()
        {
            static const std::vector<Entry> entries = [] {
                std::vector<Entry> e;
                auto add = [&e](std::initializer_list<int> types, const EffectDescriptor& d) {
                    for (int type : types)
                        e.push_back({ type, d });
                };

                // MODEL_DESAIR: short-lived, slightly oversized; rides a joint
                // and sheds feathers (see Behaviors::MoveDesair). Default render.
                add({ MODEL_DESAIR }, {
                    .create = CreateParams{ .lifeTime = 52.f, .scale = 1.4f },
                    .move = &Behaviors::MoveDesair,
                    .render = &Behaviors::RenderDefault });

                // --- Data-only effects -------------------------------------
                // These set a few creation parameters and nothing else; their
                // per-frame move/render still runs through the legacy switches.
                add({ MODEL_MAGIC_CAPSULE2 }, { .create = CreateParams{ .lifeTime = 20.f, .blendMesh = 0, .blendMeshLight = 1.0f } });
                add({ MODEL_SPEAR },          { .create = CreateParams{ .lifeTime = 10.f } });
                add({ MODEL_SUMMONER_SUMMON_NEIL_NIFE1,
                      MODEL_SUMMONER_SUMMON_NEIL_NIFE2,
                      MODEL_SUMMONER_SUMMON_NEIL_NIFE3 },
                                              { .create = CreateParams{ .lifeTime = 50.f, .scale = 1.0f, .alpha = 1.0f } });
                add({ MODEL_SUMMONER_SUMMON_NEIL_GROUND1,
                      MODEL_SUMMONER_SUMMON_NEIL_GROUND2,
                      MODEL_SUMMONER_SUMMON_NEIL_GROUND3 },
                                              { .create = CreateParams{ .lifeTime = 50.f, .scale = 1.0f, .alpha = 0.0f } });
                add({ BITMAP_FIRE_RED },      { .create = CreateParams{ .lifeTime = 40.f } });
                add({ BITMAP_IMPACT },        { .create = CreateParams{ .lifeTime = 80.f, .scale = 0.f, .blendMesh = -2 } });
                add({ BITMAP_LIGHT_MARKS },   { .create = CreateParams{ .lifeTime = 65.f } });
                add({ MODEL_PROTECT },        { .create = CreateParams{ .lifeTime = 10000.f, .velocity = 0.3f, .blendMesh = 0 } });
                add({ MODEL_CURSEDTEMPLE_HOLYITEM,
                      MODEL_CURSEDTEMPLE_PRODECTION_SKILL,
                      MODEL_CURSEDTEMPLE_RESTRAINT_SKILL },
                                              { .create = CreateParams{ .lifeTime = 9999999.f } });
                add({ MODEL_MAGIC1 },         { .create = CreateParams{ .lifeTime = 20.f, .blendMesh = 0 } });
                add({ MODEL_SKILL_FISSURE },  { .create = CreateParams{ .lifeTime = 20.f } });
                add({ MODEL_FISSURE, MODEL_FISSURE_LIGHT },
                                              { .create = CreateParams{ .lifeTime = 120.f, .scale = 0.8f } });
                add({ MODEL_BALGAS_SKILL },   { .create = CreateParams{ .lifeTime = 20.f, .scale = 1.0f, .blendMesh = 0 } });
                add({ MODEL_MAYASTAR },       { .create = CreateParams{ .lifeTime = 50.f, .scale = 50.0f } });
                add({ MODEL_BLOOD },          { .create = CreateParams{ .lifeTime = 10.f, .blendMesh = 0 } });
                add({ MODEL_POISON },         { .create = CreateParams{ .lifeTime = 40.f, .scale = 1.0f, .blendMesh = 1 } });
                add({ BITMAP_SWORDEFF },      { .create = CreateParams{ .lifeTime = 200.f } });
                add({ BITMAP_FIRE },          { .create = CreateParams{ .lifeTime = 1000.f } });
                add({ BATTLE_CASTLE_WALL1, BATTLE_CASTLE_WALL2, BATTLE_CASTLE_WALL3, BATTLE_CASTLE_WALL4 },
                                              { .create = CreateParams{ .lifeTime = 2.f } });
                add({ MODEL_INFINITY_ARROW4 },
                                              { .create = CreateParams{ .lifeTime = 15.f, .scale = 1.f, .light = std::array<float, 3>{ 1.f, 0.5f, 0.3f }, .copyLightToDirection = true } });
                add({ MODEL_CUNDUN_GHOST },
                                              { .create = CreateParams{ .lifeTime = 200.f, .scale = 1.80f, .velocity = 0.08f, .blendMesh = -2, .light = std::array<float, 3>{ 0.5f, 0.5f, 0.5f } } });

                return e;
            }();
            return entries;
        }

        // Type-indexed lookup table built once from Entries(). Pointers are
        // stable because Entries() holds a single static vector that is never
        // mutated after construction.
        const std::vector<const EffectDescriptor*>& Table()
        {
            static const std::vector<const EffectDescriptor*> table = [] {
                const auto& entries = Entries();
                int maxType = -1;
                for (const auto& entry : entries)
                    maxType = (entry.type > maxType) ? entry.type : maxType;

                std::vector<const EffectDescriptor*> t(maxType + 1, nullptr);
                for (const auto& entry : entries)
                    t[entry.type] = &entry.descriptor;
                return t;
            }();
            return table;
        }
    }

    const EffectDescriptor* Lookup(int type)
    {
        const auto& table = Table();
        if (type < 0 || type >= (int)table.size())
            return nullptr;
        return table[type];
    }
}
