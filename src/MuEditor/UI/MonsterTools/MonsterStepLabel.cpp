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

std::string Kind(const ShownValues::ShownMonster& shown)
{
    return I18N::Format(I18N::Editor::StepKind, {ShownValues::LogName(shown)});
}

std::string Name(const Data::Monsters::MonsterDefinition& monster, const char* locale)
{
    return I18N::Format(I18N::Editor::StepMonsterName, {LogName(monster), I18N::GetLanguageDisplayName(locale)});
}

std::string Attacks(const ShownValues::ShownMonster& shown)
{
    return I18N::Format(I18N::Editor::StepAttacks, {ShownValues::LogName(shown)});
}

std::string Model(const std::string& slot, const std::string& what)
{
    return I18N::Format(I18N::Editor::StepModel, {slot, what});
}

std::string SoundSetOf(const ShownValues::ShownMonster& shown)
{
    return I18N::Format(I18N::Editor::StepMonsterSoundSet, {ShownValues::LogName(shown)});
}

std::string SoundSet(const std::string& name, const std::string& what)
{
    return I18N::Format(I18N::Editor::StepSoundSet, {name, what});
}

std::string AddNumber(int number, const std::string& monster)
{
    return I18N::Format(I18N::Editor::StepAddNumber, {std::to_string(number), monster});
}

std::string CopyNumber(const Data::Monsters::MonsterDefinition& source, int number)
{
    return I18N::Format(I18N::Editor::StepCopyNumber, {LogName(source), std::to_string(number)});
}

std::string MoveNumber(const Data::Monsters::MonsterDefinition& number, const std::string& monster)
{
    return I18N::Format(I18N::Editor::StepMoveNumber, {LogName(number), monster});
}

std::string SplitNumber(const Data::Monsters::MonsterDefinition& number, const std::string& monster)
{
    return I18N::Format(I18N::Editor::StepSplitNumber, {LogName(number), monster});
}

std::string RemoveMonsterWithNumbers(const std::string& monster, int numberCount)
{
    return I18N::Format(I18N::Editor::StepRemoveMonsterWithNumbers, {monster, std::to_string(numberCount)});
}

std::string Maps(const Data::Monsters::MonsterDefinition& number)
{
    return I18N::Format(I18N::Editor::StepMaps, {LogName(number)});
}
} // namespace MuEditor::Monsters::StepLabel

#endif // _EDITOR
