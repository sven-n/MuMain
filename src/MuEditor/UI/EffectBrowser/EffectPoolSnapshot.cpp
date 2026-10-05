#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPoolSnapshot.h"

#include "GameLogic/Skills/SkillEffectMgr.h"
#include "Render/Effects/ZzzEffect.h"

#include <cstddef>

namespace MuEditor::Effects
{
namespace
{
template <typename Object, typename Entry> void Save(std::span<Object> pool, std::vector<Entry>& entries)
{
    entries.resize(pool.size());
    for (size_t i = 0; i < pool.size(); ++i)
    {
        entries[i] = {pool[i].Live, pool[i].Type};
    }
}

template <typename Object, typename Entry>
void AppendNew(EffectPool id, std::span<Object> pool, const std::vector<Entry>& entries,
               std::vector<EffectPoolSlot>& slots)
{
    for (size_t i = 0; i < pool.size(); ++i)
    {
        if (!pool[i].Live)
            continue;
        const bool wasSame = i < entries.size() && entries[i].live && entries[i].type == pool[i].Type;
        if (!wasSame)
            slots.push_back({id, static_cast<int>(i), pool[i].Type});
    }
}

size_t IndexOf(EffectPool pool)
{
    return static_cast<size_t>(pool);
}
} // namespace

EffectPools GetGamePools()
{
    return {std::span<OBJECT>(::Effects, MAX_EFFECTS),
            std::span<OBJECT>(g_SkillEffects.GetEffect(0), static_cast<size_t>(g_SkillEffects.GetSize())),
            std::span<OBJECT>(::Sprites, MAX_SPRITES), std::span<PARTICLE>(::Particles, MAX_PARTICLES),
            std::span<JOINT>(::Joints, MAX_JOINTS)};
}

void EffectPoolSnapshot::Take(const EffectPools& pools)
{
    Save(pools.effects, m_entries[IndexOf(EffectPool::Effect)]);
    Save(pools.skillEffects, m_entries[IndexOf(EffectPool::SkillEffect)]);
    Save(pools.sprites, m_entries[IndexOf(EffectPool::Sprite)]);
    Save(pools.particles, m_entries[IndexOf(EffectPool::Particle)]);
    Save(pools.joints, m_entries[IndexOf(EffectPool::Joint)]);
}

std::vector<EffectPoolSlot> EffectPoolSnapshot::NewSince(const EffectPools& pools) const
{
    std::vector<EffectPoolSlot> slots;
    AppendNew(EffectPool::Effect, pools.effects, m_entries[IndexOf(EffectPool::Effect)], slots);
    AppendNew(EffectPool::SkillEffect, pools.skillEffects, m_entries[IndexOf(EffectPool::SkillEffect)], slots);
    AppendNew(EffectPool::Sprite, pools.sprites, m_entries[IndexOf(EffectPool::Sprite)], slots);
    AppendNew(EffectPool::Particle, pools.particles, m_entries[IndexOf(EffectPool::Particle)], slots);
    AppendNew(EffectPool::Joint, pools.joints, m_entries[IndexOf(EffectPool::Joint)], slots);
    return slots;
}
} // namespace MuEditor::Effects

#endif // _EDITOR
