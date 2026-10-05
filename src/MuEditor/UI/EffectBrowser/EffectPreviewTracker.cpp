#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPreviewTracker.h"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <functional>

namespace MuEditor::Effects
{
namespace
{
constexpr int NotKept = INT_MIN;

size_t IndexOf(EffectPool pool)
{
    return static_cast<size_t>(pool);
}

template <typename Object> bool IsLiveAs(std::span<Object> pool, const EffectPoolSlot& slot)
{
    const auto index = static_cast<size_t>(slot.index);
    return index < pool.size() && pool[index].Live && pool[index].Type == slot.type;
}

bool IsLiveAs(const EffectPools& pools, const EffectPoolSlot& slot)
{
    switch (slot.pool)
    {
    case EffectPool::Effect:
        return IsLiveAs(pools.effects, slot);
    case EffectPool::SkillEffect:
        return IsLiveAs(pools.skillEffects, slot);
    case EffectPool::Sprite:
        return IsLiveAs(pools.sprites, slot);
    case EffectPool::Particle:
        return IsLiveAs(pools.particles, slot);
    case EffectPool::Joint:
        break;
    }
    return IsLiveAs(pools.joints, slot);
}

// The object a slot's object belongs to: the owner of an effect, the target
// of a particle or a joint. A sprite's owner is never used.
const OBJECT* FollowedObject(const EffectPools& pools, const EffectPoolSlot& slot)
{
    const auto index = static_cast<size_t>(slot.index);
    switch (slot.pool)
    {
    case EffectPool::Effect:
        return pools.effects[index].Owner;
    case EffectPool::SkillEffect:
        return pools.skillEffects[index].Owner;
    case EffectPool::Particle:
        return pools.particles[index].Target;
    case EffectPool::Joint:
        return pools.joints[index].Target;
    case EffectPool::Sprite:
        break;
    }
    return nullptr;
}

// The index of `object` in `pool`, or -1 when it is not one of its slots.
int SlotOf(std::span<OBJECT> pool, const OBJECT* object)
{
    const std::less<const OBJECT*> below;
    if (pool.empty() || below(object, pool.data()) || !below(object, pool.data() + pool.size()))
        return -1;
    return static_cast<int>(object - pool.data());
}
} // namespace

void EffectPreviewTracker::Fit(const EffectPools& pools)
{
    m_keptTypes[IndexOf(EffectPool::Effect)].resize(pools.effects.size(), NotKept);
    m_keptTypes[IndexOf(EffectPool::SkillEffect)].resize(pools.skillEffects.size(), NotKept);
    m_keptTypes[IndexOf(EffectPool::Sprite)].resize(pools.sprites.size(), NotKept);
    m_keptTypes[IndexOf(EffectPool::Particle)].resize(pools.particles.size(), NotKept);
    m_keptTypes[IndexOf(EffectPool::Joint)].resize(pools.joints.size(), NotKept);
}

int EffectPreviewTracker::KeptType(EffectPool pool, int index) const
{
    const std::vector<int>& kept = m_keptTypes[IndexOf(pool)];
    const auto at = static_cast<size_t>(index);
    return at < kept.size() ? kept[at] : NotKept;
}

// A kept slot that now holds another type ended and was filled again by its
// follower: the entry takes the new type.
bool EffectPreviewTracker::Keep(const EffectPoolSlot& slot)
{
    std::vector<int>& kept = m_keptTypes[IndexOf(slot.pool)];
    const auto index = static_cast<size_t>(slot.index);
    if (index >= kept.size() || kept[index] == slot.type)
        return false;
    if (kept[index] != NotKept)
    {
        const auto same = [&](const EffectPoolSlot& entry)
        { return entry.pool == slot.pool && entry.index == slot.index; };
        std::erase_if(m_kept, same);
        std::erase_if(m_created, same);
    }
    kept[index] = slot.type;
    m_kept.push_back(slot);
    return true;
}

bool EffectPreviewTracker::IsKept(const EffectPoolSlot& slot) const
{
    return KeptType(slot.pool, slot.index) == slot.type;
}

// An effect that ended in this frame still owns the last objects it created;
// one the game put into its slot since, with another type, owns nothing of
// the preview's.
bool EffectPreviewTracker::IsKeptEffect(const EffectPools& pools, const OBJECT* object) const
{
    if (object == nullptr)
        return false;
    const auto isKeptIn = [&](EffectPool pool, std::span<OBJECT> objects)
    {
        const int index = SlotOf(objects, object);
        if (index < 0)
            return false;
        const int kept = KeptType(pool, index);
        const OBJECT& slot = objects[static_cast<size_t>(index)];
        return kept != NotKept && (!slot.Live || slot.Type == kept);
    };
    return isKeptIn(EffectPool::Effect, pools.effects) || isKeptIn(EffectPool::SkillEffect, pools.skillEffects);
}

void EffectPreviewTracker::BeginCreate(const EffectPools& pools)
{
    Fit(pools);
    m_beforeCall.Take(pools);
}

int EffectPreviewTracker::EndCreate(const EffectPools& pools)
{
    const std::vector<EffectPoolSlot> filled = m_beforeCall.NewSince(pools);
    m_created.clear();
    for (const EffectPoolSlot& slot : filled)
    {
        if (slot.pool == EffectPool::Sprite)
            continue;
        Keep(slot);
        m_created.push_back(slot);
    }
    m_lastUpdate.Take(pools);
    return static_cast<int>(filled.size());
}

// Followers of followers are taken in too.
void EffectPreviewTracker::TakeInFollowers(const EffectPools& pools)
{
    if (m_kept.empty())
        return;
    const std::vector<EffectPoolSlot> candidates = m_lastUpdate.NewSince(pools);
    bool tookIn = true;
    while (tookIn)
    {
        tookIn = false;
        for (const EffectPoolSlot& slot : candidates)
        {
            if (!IsKept(slot) && IsKeptEffect(pools, FollowedObject(pools, slot)) && Keep(slot))
                tookIn = true;
        }
    }
}

// After taking in the followers: an effect that ended in this frame may
// have created its last objects first.
void EffectPreviewTracker::ForgetEnded(const EffectPools& pools)
{
    std::erase_if(m_kept,
                  [&](const EffectPoolSlot& slot)
                  {
                      if (IsLiveAs(pools, slot))
                          return false;
                      m_keptTypes[IndexOf(slot.pool)][static_cast<size_t>(slot.index)] = NotKept;
                      return true;
                  });
    std::erase_if(m_created, [&](const EffectPoolSlot& slot) { return !IsLiveAs(pools, slot); });
}

void EffectPreviewTracker::Update(const EffectPools& pools)
{
    Fit(pools);
    TakeInFollowers(pools);
    ForgetEnded(pools);
    m_lastUpdate.Take(pools);
}

bool EffectPreviewTracker::AnyCreatedLive(const EffectPools& pools) const
{
    return std::any_of(m_created.begin(), m_created.end(),
                       [&](const EffectPoolSlot& slot) { return IsLiveAs(pools, slot); });
}

void EffectPreviewTracker::RemoveAll(const EffectPools& pools, RemoveEffect removeEffect)
{
    Fit(pools);
    TakeInFollowers(pools);
    for (const EffectPoolSlot& slot : m_kept)
    {
        if (!IsLiveAs(pools, slot))
            continue;
        const auto index = static_cast<size_t>(slot.index);
        switch (slot.pool)
        {
        case EffectPool::Effect:
            removeEffect(pools.effects[index]);
            break;
        case EffectPool::SkillEffect:
            removeEffect(pools.skillEffects[index]);
            break;
        case EffectPool::Particle:
            pools.particles[index].Live = false;
            pools.particles[index].Target = nullptr;
            break;
        case EffectPool::Joint:
            pools.joints[index].Live = false;
            break;
        case EffectPool::Sprite:
            break;
        }
    }
    Forget();
}

void EffectPreviewTracker::Forget()
{
    m_kept.clear();
    m_created.clear();
    for (std::vector<int>& kept : m_keptTypes)
    {
        std::fill(kept.begin(), kept.end(), NotKept);
    }
}

EffectPreviewCounts EffectPreviewTracker::Count() const
{
    EffectPreviewCounts counts;
    for (const EffectPoolSlot& slot : m_kept)
    {
        if (slot.pool == EffectPool::Effect || slot.pool == EffectPool::SkillEffect)
            ++counts.effects;
        else if (slot.pool == EffectPool::Particle)
            ++counts.particles;
        else if (slot.pool == EffectPool::Joint)
            ++counts.joints;
    }
    return counts;
}
} // namespace MuEditor::Effects

#endif // _EDITOR
