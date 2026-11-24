#include "stdafx.h"

#include "UI/Character/CommandHintTexts.h"

#include "UI/Character/CommandHint.h"

namespace UI::Character
{
bool CommandHintTexts::Update(const GameLogic::Character::CommandEffects& effects)
{
    if (m_BuiltEffects == effects)
    {
        return false;
    }
    m_BuiltEffects = effects;
    m_Lines = BuildCommandHint(effects);
    return true;
}
} // namespace UI::Character
