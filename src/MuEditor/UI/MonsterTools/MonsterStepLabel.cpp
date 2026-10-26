#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterStepLabel.h"

#include "I18N/All.h"

namespace MuEditor::Monsters::StepLabel
{
std::string LogName(const Data::Monsters::MonsterDefinition& monster)
{
    return monster.names.GetNeutral() + " (" + std::to_string(monster.number) + ")";
}

std::string Add(const Data::Monsters::MonsterDefinition& monster)
{
    return I18N::Format(I18N::Editor::StepAdd, {LogName(monster)});
}

std::string Remove(const Data::Monsters::MonsterDefinition& monster)
{
    return I18N::Format(I18N::Editor::StepRemove, {LogName(monster)});
}

std::string Kind(const Data::Monsters::MonsterDefinition& monster)
{
    return I18N::Format(I18N::Editor::StepKind, {LogName(monster)});
}

std::string Name(const Data::Monsters::MonsterDefinition& monster, const char* locale)
{
    return I18N::Format(I18N::Editor::StepMonsterName, {LogName(monster), I18N::GetLanguageDisplayName(locale)});
}

std::string Attacks(const Data::Monsters::MonsterDefinition& monster)
{
    return I18N::Format(I18N::Editor::StepAttacks, {LogName(monster)});
}

std::string Model(const std::string& slot, const std::string& what)
{
    return I18N::Format(I18N::Editor::StepModel, {slot, what});
}
} // namespace MuEditor::Monsters::StepLabel

#endif // _EDITOR
