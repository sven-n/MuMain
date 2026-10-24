#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <vector>

namespace MuEditor
{
class IssueSubjects;

// The problems a checks tool found (Item Checks, Monster Checks): errors
// first, a box for each kind with its count that shows or hides it, and a
// table of the kind, the subject (IssueSubjects), the field and the message.
// A click on a row selects its subject and opens the tool of its field.
class IssueTable
{
public:
    // The problems of a check, errors first, each kind in the order the checks
    // found them. No row is clicked then.
    void SetIssues(std::vector<Data::Items::ItemDataIssue> issues);

    // The "Errors: 2" and "Warnings: 5" boxes, side by side.
    void RenderKindBoxes();
    // "No errors or warnings", or the table of the problems the boxes show.
    void RenderTable(IssueSubjects& subjects);

    // That the data changed since the problems were found.
    static void RenderChangedNote(const char* text);
    // What Save does with the problems, and what a click on one does.
    static void RenderNotes(const char* saveNote, const char* clickNote);

private:
    static constexpr int NoIssue = -1;

    void FilterIssues();
    void RenderRow(IssueSubjects& subjects, int issueIndex);

    std::vector<Data::Items::ItemDataIssue> m_issues;
    // Indexes into m_issues, in its order.
    std::vector<int> m_shownIssues;
    int m_errorCount = 0;
    int m_warningCount = 0;
    bool m_showErrors = true;
    bool m_showWarnings = true;
    // The index into m_issues of the problem clicked last, or NoIssue.
    int m_clickedIssue = NoIssue;
};
} // namespace MuEditor

#endif // _EDITOR
