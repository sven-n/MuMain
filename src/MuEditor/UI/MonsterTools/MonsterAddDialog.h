#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterKind.h"

#include <string>

namespace MuEditor::Monsters
{
// The dialog that adds a monster: a free number (0-1023, no entry), the
// English name and the kind. Copy starts from the selected monster: its kind
// and English name, the next free number after it; the translations stay with
// it.
class MonsterAddDialog
{
public:
    void OpenAdd();
    void OpenCopy(int sourceNumber);
    // The dialog while it is open; the added monster is selected.
    void Render();

private:
    enum class Mode
    {
        Add,
        Copy,
    };

    void RenderNumber();
    void RenderKind();
    bool CanConfirm() const;
    void Confirm();

    Mode m_mode = Mode::Add;
    bool m_open = false;
    int m_sourceNumber = -1;
    int m_number = 0;
    std::string m_name;
    Data::Monsters::MonsterKind m_kind = Data::Monsters::MonsterKind::Monster;
    // Until the kind is picked, Add gives the number's kind of the old rules.
    bool m_kindPicked = false;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
