#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterShownValues.h"

#include "MonsterEdits.h"
#include "MonsterSelection.h"
#include "MonsterStepLabel.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"

#include <vector>

namespace MuEditor::Monsters::ShownValues
{
int GetSelectedKey()
{
    const int number = g_MonsterSelection.GetNumber();
    return g_MonsterSelection.IsWholeMonster() ? number + WholeMonsterKey : number;
}

std::optional<ShownMonster> Load(int key)
{
    const bool whole = key >= WholeMonsterKey;
    const int number = whole ? key - WholeMonsterKey : key;
    std::optional<Data::Monsters::MonsterDefinition> entry = g_MonsterDataHandler.ReadMonster(number);
    if (!entry)
    {
        return std::nullopt;
    }
    const int count = static_cast<int>(g_MonsterEdits.ReadMonsterNumbers(number).size());
    // A monster with one number shares all its values with it.
    if (!whole || count < 2)
    {
        return ShownMonster{std::move(*entry), false, count};
    }
    Data::Monsters::MonsterDefinition shared = *g_MonsterEdits.ReadSharedValues(number);
    shared.number = number;
    shared.maps = entry->maps;
    return ShownMonster{std::move(shared), true, count};
}

std::optional<ShownMonster> ReadSelected()
{
    return Load(GetSelectedKey());
}

bool Store(const std::string& label, const ShownMonster& shown)
{
    if (shown.wholeMonster)
    {
        return g_MonsterEdits.SetSharedValues(label, shown.monster.number, shown.monster);
    }
    return g_MonsterEdits.SetMonster(label, shown.monster);
}

std::string LogName(const ShownMonster& shown)
{
    if (!shown.wholeMonster)
    {
        return StepLabel::LogName(shown.monster);
    }
    return I18N::Format(I18N::Editor::MonsterLogName01,
                        {shown.monster.names.GetNeutral(), std::to_string(shown.numberCount)});
}

std::string MonsterName(int number)
{
    const Data::Monsters::MonsterDefinition* shared = g_MonsterEdits.FindSharedValues(number);
    const Data::Monsters::MonsterDefinition* values = shared != nullptr ? shared : g_MonsterDatabase.Find(number);
    return values != nullptr ? values->names.GetNeutral() : std::string();
}

std::string ListNumbers(std::span<const int> numbers)
{
    std::string list;
    for (const int number : numbers)
    {
        list += (list.empty() ? "" : ", ") + std::to_string(number);
    }
    return list;
}
} // namespace MuEditor::Monsters::ShownValues

#endif // _EDITOR
