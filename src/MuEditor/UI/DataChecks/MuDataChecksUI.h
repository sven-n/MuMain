#pragma once

#ifdef _EDITOR

#include "DataCheckArea.h"
#include "IssueTable.h"

#include <memory>
#include <vector>

namespace MuEditor::DataChecks
{
// Data Checks: the problems of all game data in one table (items, monsters,
// monster models, model files, server list, effects, looks), with a box per area and per
// level that shows or hides them. A click on a problem selects what it is
// about and opens its tool. The checks run when the window is first shown and
// on Check all.
class MuDataChecksUI
{
public:
    // The window of the editor, with every area.
    static MuDataChecksUI& GetInstance();

    // A window with these areas, in this order (tests).
    explicit MuDataChecksUI(std::vector<std::unique_ptr<DataCheckArea>> areas);

    void Render(bool* open);

    // The item or monster data changed, or the editor saved a file of the
    // other areas, since the last check.
    bool DataChangedSinceCheck() const;

private:
    bool BeginWindow(bool* open);
    void RunChecks();
    void RenderCheckLine();
    void RenderAreaBoxes();

    std::vector<std::unique_ptr<DataCheckArea>> m_areas;
    // m_areas as the table takes them.
    std::vector<DataCheckArea*> m_areaPointers;
    IssueTable m_issueTable;
    bool m_checked = false;
    // The item and monster tools' data versions when the checks ran.
    long long m_checkedItemVersion = 0;
    int m_checkedMonsterVersion = 0;
    // SavedDataFiles::GetSaveCount() and Render::Look::SaveCount() when the checks ran.
    int m_checkedFileSaves = 0;
    int m_checkedLookSaves = 0;
};
} // namespace MuEditor::DataChecks

#define g_MuDataChecksUI MuEditor::DataChecks::MuDataChecksUI::GetInstance()

#endif // _EDITOR
