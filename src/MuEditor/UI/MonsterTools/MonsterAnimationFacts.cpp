#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterAnimationFacts.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "I18N/All.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace MuEditor::Monsters::AnimationFacts
{
namespace
{
// ZzzCharacter.cpp ATTACK_IMPACT_FRAME.
constexpr float AttackImpactFrame = 5.0f;
constexpr float TransformedAttackFactor = 1.5f;
constexpr float IllusionOfKundunDieFactor = 4.0f;
constexpr float ErohimDieFactor = 0.5f;
} // namespace

float GetPriorFrame(float frame, int playedFrames, bool holdsLastFrame)
{
    const int key = static_cast<int>(frame);
    if (key > 0)
    {
        return static_cast<float>(key - 1);
    }
    return holdsLastFrame ? 0.0f : static_cast<float>(std::max(playedFrames - 1, 0));
}

bool IsMonsterRange(int slot)
{
    return Data::Monsters::IsMonsterRangeSlot(slot);
}

std::optional<float> GetImpactFrame(int slot, int animation)
{
    if (IsMonsterRange(slot) && (animation == MONSTER01_ATTACK1 || animation == MONSTER01_ATTACK2))
    {
        return AttackImpactFrame;
    }
    return std::nullopt;
}

std::optional<int> GetNextAnimation(int slot, int animation)
{
    if (!IsMonsterRange(slot))
    {
        return std::nullopt;
    }
    switch (animation)
    {
    case MONSTER01_STOP2:
    case MONSTER01_SHOCK:
    case MONSTER01_ATTACK1:
    case MONSTER01_ATTACK2:
    case MONSTER01_ATTACK3:
    case MONSTER01_ATTACK4:
    case MONSTER01_ATTACK5:
        return MONSTER01_STOP1;
    case MONSTER01_APEAR:
        if (slot == MODEL_MAYA_HAND_LEFT || slot == MODEL_MAYA_HAND_RIGHT || slot == MODEL_SELUPAN)
        {
            return MONSTER01_STOP1;
        }
        return std::nullopt;
    default:
        return std::nullopt;
    }
}

std::vector<SpeedFactor> GetSpeedFactors(int slot, int animation)
{
    std::vector<SpeedFactor> factors;
    if (animation == MONSTER01_ATTACK1 || animation == MONSTER01_ATTACK2)
    {
        factors.push_back({TransformedAttackFactor, SpeedFactor::When::Transformed});
    }
    if (slot == MODEL_ILLUSION_OF_KUNDUN && animation == MONSTER01_DIE)
    {
        factors.push_back({IllusionOfKundunDieFactor, SpeedFactor::When::AfterFrame6});
    }
    if (slot == MODEL_EROHIM && animation == MONSTER01_DIE)
    {
        factors.push_back({ErohimDieFactor, SpeedFactor::When::Always});
    }
    return factors;
}

std::string DescribeInGame(int slot, int animation)
{
    std::vector<std::string> parts;
    for (const SpeedFactor& factor : GetSpeedFactors(slot, animation))
    {
        char number[16];
        std::snprintf(number, sizeof(number), "%g", factor.factor);
        const char* format = factor.when == SpeedFactor::When::Transformed   ? I18N::Editor::Speed0WhenTransformed
                             : factor.when == SpeedFactor::When::AfterFrame6 ? I18N::Editor::Speed0AfterFrame6
                                                                             : I18N::Editor::Speed0;
        parts.push_back(I18N::Format(format, {number}));
    }
    if (const std::optional<int> next = GetNextAnimation(slot, animation))
    {
        parts.push_back(I18N::Format(I18N::Editor::Then0, {Data::Monsters::GetMonsterAnimationName(*next)}));
    }
    if (const std::optional<float> impact = GetImpactFrame(slot, animation))
    {
        parts.push_back(I18N::Format(I18N::Editor::HitAtFrame0, {std::to_string(static_cast<int>(*impact))}));
    }
    std::string text;
    for (const std::string& part : parts)
    {
        text += (text.empty() ? "" : "; ") + part;
    }
    return text;
}
} // namespace MuEditor::Monsters::AnimationFacts

#endif // _EDITOR
