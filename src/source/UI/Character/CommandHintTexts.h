#pragma once

#include "GameLogic/Character/CommandEffects.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <optional>
#include <vector>

namespace UI::Character
{
// The command row's hint (CommandHint.h), formatted again only when what it shows changes: the command's effects, or
// a text it shows. Its texts name the pets by their item names ({item:13,5}), which change with the language,
// "Translate item names", the item editor and a reload of the data; the window that shows it calls Invalidate then.
class CommandHintTexts
{
public:
    // Formats the hint again when `effects` differ from what it was built from or the texts were invalidated; returns
    // whether it did.
    bool Update(const GameLogic::Character::CommandEffects& effects);
    // A text the hint shows changed: the next Update formats it again.
    void Invalidate()
    {
        m_BuiltEffects.reset();
    }

    const std::vector<UI::RmlBridge::Tooltip::Line>& Lines() const
    {
        return m_Lines;
    }

private:
    std::optional<GameLogic::Character::CommandEffects> m_BuiltEffects;
    std::vector<UI::RmlBridge::Tooltip::Line> m_Lines;
};
} // namespace UI::Character
