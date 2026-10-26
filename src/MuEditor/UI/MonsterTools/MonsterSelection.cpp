#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterSelection.h"

namespace MuEditor::Monsters
{
MonsterSelection& MonsterSelection::GetInstance()
{
    static MonsterSelection instance;
    return instance;
}

void MonsterSelection::Select(int number)
{
    m_number = number;
    ++m_serial;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
