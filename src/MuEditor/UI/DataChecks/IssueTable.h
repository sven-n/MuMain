#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <array>
#include <span>
#include <vector>

namespace MuEditor::DataChecks
{
class DataCheckArea;

// One problem a check found, and the area that found it (its index in the
// window's areas).
struct AreaIssue
{
    Data::Items::ItemDataIssue issue;
    int area = 0;
};

// The problems of the Data Checks window: the worst first (fatal, error,
// warning), a box for each level with its count that shows or hides it, a
// table of the level, the area, what the problem is about, the field and the
// message, and the areas shown. A click on a row selects what it is about and
// opens the tool of its field.
class IssueTable
{
public:
    // Fatal, Error, Warning.
    static constexpr int LevelCount = 3;

    // The problems of a check, the worst first, each level in the order the
    // checks found them. No row is clicked then.
    void SetIssues(std::vector<AreaIssue> issues);

    // Shows or hides the problems of an area; the counts follow.
    void SetAreaShown(int area, bool shown);
    bool IsAreaShown(int area) const;

    // How many problems of this level the shown areas have.
    int GetCount(Data::Items::ItemDataIssueSeverity severity) const;

    // The "Fatal: 1", "Errors: 2" and "Warnings: 5" boxes, side by side.
    void RenderKindBoxes();
    // A message that there are no problems, or the table of the problems the
    // boxes show; `areas` are the window's areas by index.
    void RenderTable(std::span<DataCheckArea* const> areas);

    // That the data changed since the problems were found.
    static void RenderChangedNote(const char* text);

private:
    static constexpr int NoIssue = -1;

    void FilterIssues();
    void RenderRow(std::span<DataCheckArea* const> areas, int issueIndex);

    std::vector<AreaIssue> m_issues;
    // Indexes into m_issues, in its order.
    std::vector<int> m_shownIssues;
    // By level, worst first; of the shown areas.
    std::array<int, LevelCount> m_counts{};
    std::array<bool, LevelCount> m_shownLevels{true, true, true};
    // By area index; an area past the end is shown.
    std::vector<bool> m_shownAreas;
    // The index into m_issues of the problem clicked last, or NoIssue.
    int m_clickedIssue = NoIssue;
};
} // namespace MuEditor::DataChecks

#endif // _EDITOR
