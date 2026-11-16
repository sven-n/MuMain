#pragma once

#include "GameLogic/Character/CommandEffects.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <optional>
#include <vector>

namespace UI::Character
{
// The command row's hint (CommandHint.h), formatted again only when what it shows changes: the command's effects,
// the language, or a text it shows. Its texts name the pets by their item names ({item:13,5}), which change with
// "Translate item names", the item editor and a reload of the data, so it follows the item texts' refresh as well as
// the language. Main thread only, like the observers it registers.
class CommandHintTexts
{
public:
    CommandHintTexts();
    ~CommandHintTexts();
    CommandHintTexts(const CommandHintTexts&) = delete;
    CommandHintTexts& operator=(const CommandHintTexts&) = delete;

    // Formats the hint again when `effects` differ from what it was built from or a text it shows changed; returns
    // whether it did.
    bool Update(const GameLogic::Character::CommandEffects& effects);

    const std::vector<UI::RmlBridge::Tooltip::Line>& Lines() const
    {
        return m_Lines;
    }

private:
    static void OnTextsChanged(void* context) noexcept;

    std::optional<GameLogic::Character::CommandEffects> m_BuiltEffects;
    bool m_TextsChanged = false;
    std::vector<UI::RmlBridge::Tooltip::Line> m_Lines;
};
} // namespace UI::Character
