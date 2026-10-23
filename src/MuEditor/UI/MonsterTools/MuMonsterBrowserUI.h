#pragma once

#ifdef _EDITOR

namespace MuEditor::Monsters
{
// The Monster Browser: the monster numbers with their names, the one place to
// select the monster the monster tools show, and Save and Revert of the
// monster list.
class MuMonsterBrowserUI
{
public:
    static MuMonsterBrowserUI& GetInstance();

    void Render(bool* open);

private:
    MuMonsterBrowserUI() = default;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterBrowserUI MuEditor::Monsters::MuMonsterBrowserUI::GetInstance()

#endif // _EDITOR
