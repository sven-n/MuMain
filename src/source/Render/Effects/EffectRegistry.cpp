#include "stdafx.h"
#include "Engine/Object/ZzzObject.h"
#include "EffectRegistry.h"
#include "Behaviors/EffectBehaviors.h"

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

                // MODEL_DESAIR: short-lived, slightly oversized; rides a joint
                // and sheds feathers (see Behaviors::MoveDesair). Default render.
                {
                    EffectDescriptor d;
                    d.create.lifeTime = 52.f;
                    d.create.scale = 1.4f;
                    d.move = &Behaviors::MoveDesair;
                    d.render = &Behaviors::RenderDefault;
                    e.push_back({ MODEL_DESAIR, d });
                }

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
