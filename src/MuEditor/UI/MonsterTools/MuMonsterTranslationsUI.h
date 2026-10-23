#pragma once

#ifdef _EDITOR

#include "MonsterNameRows.h"

#include "../MuEditor/UI/Common/NamesGrid.h"

#include <string>

namespace MuEditor::Monsters
{
// Monster Translations: the monster names as a grid of monsters and the
// languages of the UI (English first), with how many names each language
// misses and a filter for them (the names grid), and the import and export of
// the original client's NpcName txt files. Edits go into the monster table
// right away and reach the monsters in the game; Save Monsters writes them.
class MuMonsterTranslationsUI
{
public:
    static MuMonsterTranslationsUI& GetInstance();

    void Render(bool* open);

private:
    MuMonsterTranslationsUI() = default;

    void RenderFileActions();
    void ImportNames();
    void ExportNames();

    NamesGrid m_grid;
    MonsterNameRows m_rows;
    // What the last import or export did, in short; the console has the rest.
    std::string m_status;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterTranslationsUI MuEditor::Monsters::MuMonsterTranslationsUI::GetInstance()

#endif // _EDITOR
