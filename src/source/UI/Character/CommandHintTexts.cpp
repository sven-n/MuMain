#include "stdafx.h"

#include "UI/Character/CommandHintTexts.h"

#include "Data/Translation/ItemTextExpansion.h"
#include "I18N/All.h"
#include "UI/Character/CommandHint.h"

namespace UI::Character
{
CommandHintTexts::CommandHintTexts()
{
    I18N::RegisterLocaleObserver(&CommandHintTexts::OnTextsChanged, this);
    Data::Translation::ItemTexts::RegisterTextsChangedObserver(&CommandHintTexts::OnTextsChanged, this);
}

CommandHintTexts::~CommandHintTexts()
{
    I18N::UnregisterLocaleObserver(&CommandHintTexts::OnTextsChanged, this);
    Data::Translation::ItemTexts::UnregisterTextsChangedObserver(&CommandHintTexts::OnTextsChanged, this);
}

void CommandHintTexts::OnTextsChanged(void* context) noexcept
{
    static_cast<CommandHintTexts*>(context)->m_TextsChanged = true;
}

bool CommandHintTexts::Update(const GameLogic::Character::CommandEffects& effects)
{
    if (m_BuiltEffects == effects && !m_TextsChanged)
    {
        return false;
    }
    m_BuiltEffects = effects;
    m_TextsChanged = false;
    m_Lines = BuildCommandHint(effects);
    return true;
}
} // namespace UI::Character
