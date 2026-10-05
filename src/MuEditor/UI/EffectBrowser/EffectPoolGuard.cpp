#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPoolGuard.h"

#include "Render/Effects/ZzzEffect.h"

#include <cstddef>

namespace MuEditor::Effects
{
namespace
{
template <typename Pool, size_t Count> void SaveLive(const Pool* pool, std::array<bool, Count>& live)
{
    for (size_t i = 0; i < Count; ++i)
    {
        live[i] = pool[i].Live;
    }
}

template <typename Pool, size_t Count> void RemoveNew(Pool* pool, const std::array<bool, Count>& live)
{
    for (size_t i = 0; i < Count; ++i)
    {
        if (pool[i].Live && !live[i])
            pool[i].Live = false;
    }
}
} // namespace

EffectPoolGuard::EffectPoolGuard()
{
    SaveLive(::Effects, m_effects);
    SaveLive(::Sprites, m_sprites);
    SaveLive(::Particles, m_particles);
    SaveLive(::Joints, m_joints);
}

EffectPoolGuard::~EffectPoolGuard()
{
    RemoveNew(::Effects, m_effects);
    RemoveNew(::Sprites, m_sprites);
    RemoveNew(::Particles, m_particles);
    RemoveNew(::Joints, m_joints);
}
} // namespace MuEditor::Effects

#endif // _EDITOR
