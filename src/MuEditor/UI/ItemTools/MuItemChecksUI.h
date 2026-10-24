#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/IssueTable.h"

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
    CMuItemChecksUI() = default;

    void RunChecks();
    void RenderCheckLine();

    MuEditor::IssueTable m_issueTable;
    bool m_checked = false;
    // ToolWindow::GetDataVersion() when the checks ran.
    long long m_checkedDataVersion = 0;
};

#define g_MuItemChecksUI CMuItemChecksUI::GetInstance()

#endif // _EDITOR
