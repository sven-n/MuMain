#include "stdafx.h"

#include "UI/Character/CriticalHitTexts.h"

namespace UI::Character
{
bool CriticalHitTexts::Update(const Parts& parts)
{
    if (m_Built == parts)
    {
        return false;
    }
    m_Built = parts;
    m_RateLine = BuildCriticalDamageRateLine(parts.rate);
    m_RateHintLines = BuildCriticalDamageRateHint(parts.rate);
    m_DamageHintLines = BuildDamageHint(parts.damageRange, parts.bonus, parts.excellentBonus);
    return true;
}
} // namespace UI::Character
