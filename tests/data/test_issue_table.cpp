#include "stdafx.h"

#include "doctest.h"

// The problem table of the checks tools: three levels, counted apart, the
// worst first, each row named by its level.
#ifdef _EDITOR

#include "TestImGuiContext.h"

#include "I18N/All.h"
#include "UI/Common/IssueSubjects.h"
#include "UI/Common/IssueTable.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <string>
#include <string_view>
#include <vector>

using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;

namespace
{
constexpr char WindowName[] = "Issue table test";
constexpr char TableId[] = "##Issues";
constexpr ImVec2 WindowSize(900.0f, 500.0f);

// Remembers the problem a click opened.
class RecordingSubjects final : public MuEditor::IssueSubjects
{
public:
    const char* GetHeader() const override
    {
        return "Subject";
    }

    float GetColumnWidth() const override
    {
        return 100.0f;
    }

    void RenderCell(const ItemDataIssue& issue) const override
    {
        ImGui::TextUnformatted(issue.source.c_str());
    }

    std::string_view GetField(const ItemDataIssue& issue) const override
    {
        return issue.field;
    }

    void Open(const ItemDataIssue& issue, bool) override
    {
        opened = issue.message;
    }

    std::string opened;
};

ItemDataIssue Issue(ItemDataIssueSeverity severity, const char* message)
{
    return {severity, "file.json", ItemDataIssue::NoItem, ItemDataIssue::NoItem, "field", message};
}

void RenderFrame(MuEditor::IssueTable& table, RecordingSubjects& subjects)
{
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(WindowSize);
    ImGui::Begin(WindowName);
    table.RenderKindBoxes();
    table.RenderTable(subjects);
    ImGui::End();
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

// Clicks the row at `index` (among the shown rows), named by its level.
void ClickRow(MuEditor::IssueTable& table, RecordingSubjects& subjects, int index, const char* levelLabel)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowName);
    REQUIRE(window != nullptr);
    const ImGuiID tableId = ImGui::GetIDWithSeed(TableId, nullptr, window->ID);
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed(levelLabel, nullptr, ImGui::GetIDWithSeed(index, tableId)));
    RenderFrame(table, subjects);
}
} // namespace

TEST_CASE("The issue table counts fatal problems, errors and warnings apart [editor]")
{
    MuEditor::IssueTable table;
    table.SetIssues({Issue(ItemDataIssueSeverity::Warning, "w1"), Issue(ItemDataIssueSeverity::Fatal, "f1"),
                     Issue(ItemDataIssueSeverity::Error, "e1"), Issue(ItemDataIssueSeverity::Fatal, "f2"),
                     Issue(ItemDataIssueSeverity::Warning, "w2")});

    CHECK(table.GetCount(ItemDataIssueSeverity::Fatal) == 2);
    CHECK(table.GetCount(ItemDataIssueSeverity::Error) == 1);
    CHECK(table.GetCount(ItemDataIssueSeverity::Warning) == 2);
}

TEST_CASE("The issue table shows the worst first, each level in the order found [editor]")
{
    TestImGuiContext context;
    MuEditor::IssueTable table;
    RecordingSubjects subjects;
    table.SetIssues({Issue(ItemDataIssueSeverity::Warning, "w1"), Issue(ItemDataIssueSeverity::Error, "e1"),
                     Issue(ItemDataIssueSeverity::Fatal, "f1"), Issue(ItemDataIssueSeverity::Fatal, "f2")});
    RenderFrame(table, subjects);

    ClickRow(table, subjects, 0, I18N::Editor::Fatal);
    CHECK(subjects.opened == "f1");
    ClickRow(table, subjects, 1, I18N::Editor::Fatal);
    CHECK(subjects.opened == "f2");
    ClickRow(table, subjects, 2, I18N::Editor::Error);
    CHECK(subjects.opened == "e1");
    ClickRow(table, subjects, 3, I18N::Editor::Warning);
    CHECK(subjects.opened == "w1");
}

#endif // _EDITOR
