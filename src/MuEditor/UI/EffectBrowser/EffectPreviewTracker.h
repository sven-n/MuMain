#pragma once

#ifdef _EDITOR

#include "EffectPoolSnapshot.h"

#include <array>
#include <functional>
#include <vector>

namespace MuEditor::Effects
{
struct EffectPreviewCounts
{
    int effects = 0;
    int particles = 0;
    int joints = 0;
};

// Removes an effect as the game does when its life ends, with its trails: what the previews pass to RemoveAll.
void RemoveGameEffect(OBJECT& effect);

// What the world preview created and what that created in turn, followed from
// frame to frame, to remove it all when the preview stops. Sprites are not
// kept: a sprite lives until the next RenderSprites.
class EffectPreviewTracker
{
public:
    using RemoveEffect = void (*)(OBJECT& effect);

    // Around the game's create call: the slots the call fills are the
    // preview's, whatever their owner.
    void BeginCreate(const EffectPools& pools);
    // Returns how many slots the call filled, sprites included.
    int EndCreate(const EffectPools& pools);
    // Once a frame: takes in the objects that became live since the last
    // update and are owned (effects) or targeted (particles, joints) by a kept
    // effect, then forgets the slots that ended or hold another type. A slot
    // refilled with the type it held cannot be told from it.
    void Update(const EffectPools& pools);
    // Whether a slot the last create call filled is still the preview's.
    bool AnyCreatedLive(const EffectPools& pools) const;
    // Every slot the preview keeps, sprites left out.
    const std::vector<EffectPoolSlot>& GetKept() const
    {
        return m_kept;
    }
    // The slots the last create call filled that are still the preview's,
    // sprites left out.
    const std::vector<EffectPoolSlot>& GetCreated() const
    {
        return m_created;
    }
    // Takes in the latest followers, then removes everything kept in one pass,
    // so no kept object outlives its owner. Effects go through removeEffect.
    void RemoveAll(const EffectPools& pools, RemoveEffect removeEffect);
    // Forgets everything without touching the pools.
    void Forget();
    // Forgets the slots `notOurs` says are no longer the tracked effects' (their things ended and the slot was
    // taken again), without touching them.
    void ForgetSlots(const std::function<bool(const EffectPoolSlot&)>& notOurs);

    EffectPreviewCounts Count() const;
    bool IsEmpty() const
    {
        return m_kept.empty();
    }

private:
    void Fit(const EffectPools& pools);
    bool Keep(const EffectPoolSlot& slot);
    bool IsKept(const EffectPoolSlot& slot) const;
    int KeptType(EffectPool pool, int index) const;
    bool IsKeptEffect(const EffectPools& pools, const OBJECT* object) const;
    void TakeInFollowers(const EffectPools& pools);
    void ForgetEnded(const EffectPools& pools);

    std::vector<EffectPoolSlot> m_kept;
    // The slots the last create call filled.
    std::vector<EffectPoolSlot> m_created;
    // Per pool and slot, the type of the kept entry that has it, or NotKept.
    std::array<std::vector<int>, EffectPoolCount> m_keptTypes;
    EffectPoolSnapshot m_beforeCall;
    EffectPoolSnapshot m_lastUpdate;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
