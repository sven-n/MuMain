#pragma once

#ifdef _EDITOR

namespace MuEditor::Monsters
{
// The monster the monster tools show: one selection for all of them, so
// selecting a monster in one tool opens it in the others. Any monster number
// can be selected, also one without an entry.
class MonsterSelection
{
public:
    static constexpr int NoMonster = -1;

    static MonsterSelection& GetInstance();

    int GetNumber() const
    {
        return m_number;
    }

    // Selects the monster; selecting it again counts too, so the tools show it
    // again (scroll to it, clear a filter that hides it).
    void Select(int number);

    // Counts the Select calls. A tool remembers the serial it last showed and
    // shows the selected monster once when it changes.
    int GetSerial() const
    {
        return m_serial;
    }

private:
    int m_number = NoMonster;
    int m_serial = 0;
};
} // namespace MuEditor::Monsters

#define g_MonsterSelection MuEditor::Monsters::MonsterSelection::GetInstance()

#endif // _EDITOR
