#pragma once

#ifdef _EDITOR

#include "MonsterDataUsers.h"

#include <vector>

namespace MuEditor::Monsters
{
// The dialog that removes a monster's entry, after it said what the game does
// with the number then and listed what uses the number: the quest data and the
// source lines that name it. The removal can be undone. For a monster with
// several numbers as a whole it removes all of them (D61), with the uses of
// each.
class MonsterRemoveDialog
{
public:
    void Open(int number);
    void OpenWholeMonster(int number);
    void Render();

private:
    void Open(int number, bool wholeMonster);
    // The numbers Remove removes now and their uses, read again when the
    // monsters change while the dialog is open (an undo).
    void ReadNumbers();
    void RenderUses(int number, const MonsterDataUsers& users) const;
    void RenderDataUsers(const MonsterDataUsers& users) const;

    bool m_open = false;
    int m_number = -1;
    // Asked for the whole monster, and whether it has several numbers.
    bool m_wholeRequested = false;
    bool m_wholeMonster = false;
    std::vector<int> m_numbers;
    std::vector<MonsterDataUsers> m_dataUsers;
    // The data version m_numbers was read at.
    int m_numbersVersion = -1;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
