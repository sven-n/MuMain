#pragma once

#ifdef _EDITOR

#include "MonsterDataUsers.h"

namespace MuEditor::Monsters
{
// The dialog that removes a monster's entry, after it said what the game does
// with the number then and listed what uses the number: the quest data and the
// source lines that name it. The removal can be undone.
class MonsterRemoveDialog
{
public:
    void Open(int number);
    void Render();

private:
    void RenderUses() const;
    void RenderDataUsers() const;

    bool m_open = false;
    int m_number = -1;
    MonsterDataUsers m_dataUsers;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
