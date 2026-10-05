#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPoolGuard.h"

#include <cstddef>

namespace MuEditor::Effects
{
EffectPoolGuard::EffectPoolGuard() : m_pools(GetGamePools())
{
    m_before.Take(m_pools);
}

EffectPoolGuard::~EffectPoolGuard()
{
    for (const EffectPoolSlot& slot : m_before.NewSince(m_pools))
    {
        const auto index = static_cast<size_t>(slot.index);
        switch (slot.pool)
        {
        case EffectPool::Effect:
            m_pools.effects[index].Live = false;
            break;
        case EffectPool::SkillEffect:
            m_pools.skillEffects[index].Live = false;
            break;
        case EffectPool::Sprite:
            m_pools.sprites[index].Live = false;
            break;
        case EffectPool::Particle:
            m_pools.particles[index].Live = false;
            break;
        case EffectPool::Joint:
            m_pools.joints[index].Live = false;
            break;
        }
    }
}
} // namespace MuEditor::Effects

#endif // _EDITOR
