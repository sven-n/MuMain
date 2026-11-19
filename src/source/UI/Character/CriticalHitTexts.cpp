#include "stdafx.h"

#include "UI/Character/CriticalHitTexts.h"

#include "UI/Character/HintLines.h"

namespace UI::Character
{
bool CriticalHitTexts::NamesChanged(const Parts& parts) const
{
    return m_ScrollOfBattleName != TableItemName(ITEM_SCROLL_OF_BATTLE) ||
           m_RateBuffName != TableSkillName(parts.rate.buffSkill) ||
           m_DamageBuffName != TableSkillName(parts.bonus.buffSkill);
}

bool CriticalHitTexts::Update(const Parts& parts)
{
    if (m_Built == parts && !NamesChanged(parts))
    {
        return false;
    }
    m_Built = parts;
    m_ScrollOfBattleName = TableItemName(ITEM_SCROLL_OF_BATTLE);
    m_RateBuffName = TableSkillName(parts.rate.buffSkill);
    m_DamageBuffName = TableSkillName(parts.bonus.buffSkill);
    m_RateLine = BuildCriticalDamageRateLine(parts.rate);
    m_RateHintLines = BuildCriticalDamageRateHint(parts.rate);
    m_DamageHintLines = BuildDamageHint(parts.damageRange, parts.bonus, parts.excellentBonus);
    return true;
}
} // namespace UI::Character
