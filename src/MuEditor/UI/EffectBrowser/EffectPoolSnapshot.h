#pragma once

#ifdef _EDITOR

#include "Core/Globals/_struct.h"
#include "Engine/Object/w_ObjectInfo.h"

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace MuEditor::Effects
{
// The game's pools of effect objects. Effects the character owns of a few
// skill types go to the skill effects (CreateEffect, CSkillEffectMgr), not to
// the effects.
enum class EffectPool : std::uint8_t
{
    Effect,
    SkillEffect,
    Sprite,
    Particle,
    Joint,
};

inline constexpr int EffectPoolCount = 5;

struct EffectPools
{
    std::span<OBJECT> effects;
    std::span<OBJECT> skillEffects;
    std::span<OBJECT> sprites;
    std::span<PARTICLE> particles;
    std::span<JOINT> joints;
};

// A live slot of a pool and the type it holds.
struct EffectPoolSlot
{
    EffectPool pool = EffectPool::Effect;
    int index = 0;
    int type = 0;

    bool operator==(const EffectPoolSlot&) const = default;
};

// The pools of the game.
EffectPools GetGamePools();

// Which slots of the pools are live and the type each holds, to find the
// slots filled since: a slot freed and filled again with another type counts
// as filled.
class EffectPoolSnapshot
{
public:
    void Take(const EffectPools& pools);
    // The slots live now that were not live, or held another type, when the
    // snapshot was taken.
    std::vector<EffectPoolSlot> NewSince(const EffectPools& pools) const;

private:
    struct Entry
    {
        bool live = false;
        int type = 0;
    };

    std::array<std::vector<Entry>, EffectPoolCount> m_entries;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
