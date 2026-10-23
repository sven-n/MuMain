#pragma once

#ifdef _EDITOR

namespace MuEditor::Monsters
{
// Monster Checks: the errors and warnings Save finds in the monster list.
class MuMonsterChecksUI
{
public:
    static MuMonsterChecksUI& GetInstance();

    void Render(bool* open);

private:
    MuMonsterChecksUI() = default;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterChecksUI MuEditor::Monsters::MuMonsterChecksUI::GetInstance()

#endif // _EDITOR
