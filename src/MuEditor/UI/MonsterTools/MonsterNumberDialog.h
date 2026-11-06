#pragma once

#ifdef _EDITOR

#include <string>

namespace MuEditor::Monsters
{
// The dialogs of the Monster Browser's number actions (D61): Add number and
// Copy number take a free number for the new number of the monster (Q-MN99);
// Move number to... takes the monster to move it into (Q-MN97).
class MonsterNumberDialog
{
public:
    void OpenAddNumber(int memberNumber);
    void OpenCopyNumber(int number);
    void OpenMoveNumber(int number);
    // The dialog while it is open; the new or moved number is selected.
    void Render();

private:
    enum class Mode
    {
        AddNumber,
        CopyNumber,
        MoveNumber,
    };

    void Open(Mode mode, int source);
    void RenderNumber();
    void RenderTargets();
    bool CanConfirm() const;
    void Confirm();

    Mode m_mode = Mode::AddNumber;
    bool m_open = false;
    int m_source = -1;
    int m_number = 0;
    // The monster to move into, by one of its numbers; -1 for none.
    int m_target = -1;
    std::string m_search;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
