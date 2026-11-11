#pragma once

#include "GameLogic/Character/CommandEffects.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <optional>
#include <string>
#include <vector>

// The hint of the character window's command row: what the Dark Lord's command adds (issue #1017). It is formatted
// again only when what it shows changes.
namespace UI::Character
{
// The command's parts: the PvM attack rate, the Dark Raven's damage and attack speed, and the guild members a guild
// of the Dark Lord can have.
std::vector<UI::RmlBridge::Tooltip::Line> BuildCommandHint(const GameLogic::Character::CommandEffects& effects);

class CommandHintTexts
{
public:
    // Formats the hint again when the effects, the language or the Dark Raven's item name differ from what it was built
    // from; returns whether it did.
    bool Update(const GameLogic::Character::CommandEffects& effects, const std::string& locale);

    const std::vector<UI::RmlBridge::Tooltip::Line>& Lines() const
    {
        return m_Lines;
    }

private:
    std::optional<GameLogic::Character::CommandEffects> m_BuiltEffects;
    std::string m_BuiltLocale;
    // The Dark Raven's item name the hint was built with.
    std::wstring m_RavenName;
    std::vector<UI::RmlBridge::Tooltip::Line> m_Lines;
};
} // namespace UI::Character
