#pragma once

#ifdef _EDITOR

#include "MonsterAddDialog.h"
#include "MonsterBrowserList.h"
#include "MonsterListModel.h"
#include "MonsterNumberDialog.h"
#include "MonsterRemoveDialog.h"
#include "MonsterTreeModel.h"

#include "Data/GameData/MapData/MapRef.h"

#include <string>
#include <vector>

namespace MuEditor::Monsters
{
// The Monster Browser: the monsters with their numbers (D61), names, kinds
// and maps, a search and filters, where monsters and numbers are added,
// copied, moved and removed and their kind and maps are set, and Save and
// Revert of the monster list. A click selects the monster or number for all
// monster tools.
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
    // Opens the dialog of an action of a row's context menu or a button.
    void StartAction(MonsterBrowserList::Action action, int number);
    // A kind picked in a row's combo, as an undo step: of the number, or the
    // shared kind of a monster's row.
    void SetKind(int number, bool wholeMonster, Data::Monsters::MonsterKind kind);
    // The maps picked for a number, as an undo step (Q-MN98).
    void SetMaps(int number, const std::vector<Data::Maps::MapRef>& maps);
    // The number was selected: it and the rows stay as they are.
    void SelectFromList(int number, bool wholeMonster);
    // When the selection changed elsewhere: clears the filters that hide the
    // monster; returns whether the list scrolls to it.
    bool RevealSelection(int selectedNumber);

    MonsterTreeModel m_list;
    MonsterListModel::Filter m_filter;
    std::string m_search;
    MonsterBrowserList m_rows;
    MonsterAddDialog m_addDialog;
    MonsterNumberDialog m_numberDialog;
    MonsterRemoveDialog m_removeDialog;
    int m_shownSerial = -1;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterBrowserUI MuEditor::Monsters::MuMonsterBrowserUI::GetInstance()

#endif // _EDITOR
