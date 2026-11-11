#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterSelection.h"

#include "MonsterShownModel.h"

namespace MuEditor::Monsters
{
MonsterSelection& MonsterSelection::GetInstance()
{
    static MonsterSelection instance;
    return instance;
}

void MonsterSelection::Select(int number)
{
    // Another monster shows its entry's look, also when the one before is
    // selected again (Q-MN83); the same one again (an undo) keeps its look.
    if (number != m_number)
    {
        ShownModel::SetShownVariant(NoMonster, -1);
    }
    m_number = number;
    m_wholeMonster = false;
    ++m_serial;
}

void MonsterSelection::SelectMonster(int number)
{
    Select(number);
    m_wholeMonster = true;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
