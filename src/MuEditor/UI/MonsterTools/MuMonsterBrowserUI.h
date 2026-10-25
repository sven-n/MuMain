#pragma once

#ifdef _EDITOR

#include "MonsterAddDialog.h"
#include "MonsterBrowserList.h"
#include "MonsterListModel.h"
#include "MonsterRemoveDialog.h"

#include <string>

namespace MuEditor::Monsters
{
// The Monster Browser: the monster numbers with their names and kinds, a
// search and filters, where monsters are added, copied and removed and their
// kind is set, and Save and Revert of the monster list. A click selects the
// monster for all monster tools.
class MuMonsterBrowserUI
{
public:
    static MuMonsterBrowserUI& GetInstance();

    void Render(bool* open);

private:
    MuMonsterBrowserUI() = default;

    void RenderFileActions();
    void RenderFilters();
    void RenderKindFilter();
    void RenderMissingFilter();
    void RenderMonsterActions(int selectedNumber);
    // A kind picked in a row's combo, as an undo step.
    void SetKind(int number, Data::Monsters::MonsterKind kind);
    // When the selection changed elsewhere: clears the filters that hide the
    // monster; returns whether the list scrolls to it.
    bool RevealSelection(int selectedNumber);

    MonsterListModel m_list;
    MonsterListModel::Filter m_filter;
    std::string m_search;
    MonsterBrowserList m_rows;
    MonsterAddDialog m_addDialog;
    MonsterRemoveDialog m_removeDialog;
    int m_shownSerial = -1;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterBrowserUI MuEditor::Monsters::MuMonsterBrowserUI::GetInstance()

#endif // _EDITOR
