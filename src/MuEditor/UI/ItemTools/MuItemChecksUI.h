#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <vector>

// The item checks (section 9): the errors and warnings Save Items finds in the
// item data and the item models, errors first. A click on a problem of an
// item selects the item and opens the tool of its field. The checks run when
// the window is first shown and on Check, not every frame: they write and
// read every model file in memory and look for the model files on disk.
class CMuItemChecksUI
{
public:
    static CMuItemChecksUI& GetInstance();

    void Render(bool* open);

private:
    static constexpr int NoIssue = -1;

    CMuItemChecksUI() = default;

    void RunChecks();
    // The problems the error and warning boxes show.
    void FilterIssues();
    void RenderCheckLine();
    void RenderNotes();
    void RenderTable();
    void RenderRow(int issueIndex);

    std::vector<Data::Items::ItemDataIssue> m_issues;
    // Indexes into m_issues, in its order.
    std::vector<int> m_shownIssues;
    int m_errorCount = 0;
    int m_warningCount = 0;
    bool m_checked = false;
    // ToolWindow::GetDataVersion() when the checks ran.
    long long m_checkedDataVersion = 0;
    bool m_showErrors = true;
    bool m_showWarnings = true;
    // The index into m_issues of the problem clicked last, or NoIssue.
    int m_clickedIssue = NoIssue;
};

#define g_MuItemChecksUI CMuItemChecksUI::GetInstance()

#endif // _EDITOR
