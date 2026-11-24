#pragma once

#include "GameLogic/Combat/CriticalDamageBonus.h"
#include "GameLogic/Combat/CriticalDamageRate.h"
#include "UI/Character/CriticalDamageRateLine.h"
#include "UI/Character/DamageHint.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <optional>
#include <string>
#include <vector>

// The character window's critical hit texts: the critical damage rate line and the hints of the damage line and of the
// rate line. They are formatted again only when what they show changes.
namespace UI::Character
{
class CriticalHitTexts
{
public:
    // What the texts show. A language switch or a refresh of the item texts reaches them through Invalidate (the window
    // watches for both: TextsChangedWatch); the names they show (the Scroll of Battle, the buffs' skills) are also
    // compared with the item and skill tables in place, since the skill editor changes them without either.
    struct Parts
    {
        GameLogic::Combat::CriticalDamageRate rate;
        GameLogic::Combat::CriticalDamageBonus bonus;
        int excellentBonus = 0;
        DamageRange damageRange;

        bool operator==(const Parts&) const = default;
    };

    // Formats the texts again when `parts` differ from what they were built from; returns whether it did.
    bool Update(const Parts& parts);
    // The language or an item text changed: the next Update formats the texts again.
    void Invalidate()
    {
        m_Built.reset();
    }

    const CriticalDamageRateLine& RateLine() const
    {
        return m_RateLine;
    }
    const std::vector<UI::RmlBridge::Tooltip::Line>& RateHintLines() const
    {
        return m_RateHintLines;
    }
    const std::vector<UI::RmlBridge::Tooltip::Line>& DamageHintLines() const
    {
        return m_DamageHintLines;
    }

private:
    bool NamesChanged(const Parts& parts) const;

    std::optional<Parts> m_Built;
    // The names the texts were built with.
    std::wstring m_ScrollOfBattleName;
    std::wstring m_RateBuffName;
    std::wstring m_DamageBuffName;
    CriticalDamageRateLine m_RateLine;
    std::vector<UI::RmlBridge::Tooltip::Line> m_RateHintLines;
    std::vector<UI::RmlBridge::Tooltip::Line> m_DamageHintLines;
};
} // namespace UI::Character
