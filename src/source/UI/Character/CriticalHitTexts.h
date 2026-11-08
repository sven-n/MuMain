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
    // What the texts show, in which language and with which item names (the Scroll of Battle's line names the item).
    struct Parts
    {
        GameLogic::Combat::CriticalDamageRate rate;
        GameLogic::Combat::CriticalDamageBonus bonus;
        int excellentBonus = 0;
        DamageRange damageRange;
        std::string locale;
        bool translatedItemNames = false;

        bool operator==(const Parts&) const = default;
    };

    // Formats the texts again when `parts` differ from what they were built from; returns whether it did.
    bool Update(const Parts& parts);

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
    std::optional<Parts> m_Built;
    CriticalDamageRateLine m_RateLine;
    std::vector<UI::RmlBridge::Tooltip::Line> m_RateHintLines;
    std::vector<UI::RmlBridge::Tooltip::Line> m_DamageHintLines;
};
} // namespace UI::Character
