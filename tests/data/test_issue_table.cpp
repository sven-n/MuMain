#include "stdafx.h"

#include "doctest.h"

// The problem table of Data Checks: three levels, counted apart, the worst
// first, each row named by its level and opened through its area; boxes hide
// levels and areas.
#ifdef _EDITOR

#include "TestImGuiContext.h"

#include "I18N/All.h"
#include "UI/Common/IssueLevels.h"
#include "UI/DataChecks/DataCheckArea.h"
#include "UI/DataChecks/IssueTable.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <array>
#include <string>
#include <string_view>
#include <vector>

using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using MuEditor::DataChecks::AreaIssue;
using MuEditor::DataChecks::DataCheckArea;
using MuEditor::DataChecks::IssueTable;

namespace
{
constexpr char WindowName[] = "Issue table test";
constexpr char TableId[] = "##Issues";
constexpr ImVec2 WindowSize(900.0f, 500.0f);

// Remembers the problem a click opened.
class RecordingArea final : public DataCheckArea
{
public:
    explicit RecordingArea(const char* name) : m_name(name) {}

    const char* GetName() const override
    {
        return m_name;
    }

    void Check(std::vector<ItemDataIssue>&) override {}

    void RenderSubject(const ItemDataIssue& issue) const override
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

private:
    const char* m_name;
};

AreaIssue Issue(ItemDataIssueSeverity severity, const char* message, int area = 0)
{
    return {{severity, "file.json", ItemDataIssue::NoItem, ItemDataIssue::NoItem, "field", message}, area};
}

void RenderFrame(IssueTable& table, std::span<DataCheckArea* const> areas)
{
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(WindowSize);
    ImGui::Begin(WindowName);
    table.RenderKindBoxes();
    table.RenderTable(areas);
    ImGui::End();
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

// Clicks the row of the problem at `index` (among all problems, the worst
// first), named by its level.
void ClickRow(IssueTable& table, std::span<DataCheckArea* const> areas, int index, const char* levelLabel)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowName);
    REQUIRE(window != nullptr);
    const ImGuiID tableId = ImGui::GetIDWithSeed(TableId, nullptr, window->ID);
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed(levelLabel, nullptr, ImGui::GetIDWithSeed(index, tableId)));
    RenderFrame(table, areas);
}
} // namespace

TEST_CASE("Each problem level has a name and a color of its own [editor]")
{
    using MuEditor::IssueLevels::GetColor;
    using MuEditor::IssueLevels::GetName;
    CHECK(std::string(GetName(ItemDataIssueSeverity::Fatal)) == I18N::Editor::Fatal);
    CHECK(std::string(GetName(ItemDataIssueSeverity::Error)) == I18N::Editor::Error);
    CHECK(std::string(GetName(ItemDataIssueSeverity::Warning)) == I18N::Editor::Warning);
    const auto same = [](ImVec4 left, ImVec4 right)
    { return left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w; };
    CHECK_FALSE(same(GetColor(ItemDataIssueSeverity::Fatal), GetColor(ItemDataIssueSeverity::Error)));
    CHECK_FALSE(same(GetColor(ItemDataIssueSeverity::Error), GetColor(ItemDataIssueSeverity::Warning)));
    CHECK_FALSE(same(GetColor(ItemDataIssueSeverity::Fatal), GetColor(ItemDataIssueSeverity::Warning)));
}

TEST_CASE("Data Checks' table counts fatal problems, errors and warnings apart [editor]")
{
    IssueTable table;
    table.SetIssues({Issue(ItemDataIssueSeverity::Warning, "w1"), Issue(ItemDataIssueSeverity::Fatal, "f1"),
                     Issue(ItemDataIssueSeverity::Error, "e1"), Issue(ItemDataIssueSeverity::Fatal, "f2"),
                     Issue(ItemDataIssueSeverity::Warning, "w2")});

    CHECK(table.GetCount(ItemDataIssueSeverity::Fatal) == 2);
    CHECK(table.GetCount(ItemDataIssueSeverity::Error) == 1);
    CHECK(table.GetCount(ItemDataIssueSeverity::Warning) == 2);
}

TEST_CASE("Data Checks' table shows the worst first and opens a row through its area [editor]")
{
    TestImGuiContext context;
    RecordingArea items("Items");
    RecordingArea monsters("Monsters");
    const std::array<DataCheckArea*, 2> areas = {&items, &monsters};
    IssueTable table;
    table.SetIssues({Issue(ItemDataIssueSeverity::Warning, "w1", 1), Issue(ItemDataIssueSeverity::Error, "e1", 0),
                     Issue(ItemDataIssueSeverity::Fatal, "f1", 1), Issue(ItemDataIssueSeverity::Fatal, "f2", 0)});
    RenderFrame(table, areas);

    ClickRow(table, areas, 0, I18N::Editor::Fatal);
    CHECK(monsters.opened == "f1");
    ClickRow(table, areas, 1, I18N::Editor::Fatal);
    CHECK(items.opened == "f2");
    ClickRow(table, areas, 2, I18N::Editor::Error);
    CHECK(items.opened == "e1");
    ClickRow(table, areas, 3, I18N::Editor::Warning);
    CHECK(monsters.opened == "w1");
}

TEST_CASE("Data Checks' table hides a level with its box [editor]")
{
    TestImGuiContext context;
    RecordingArea items("Items");
    const std::array<DataCheckArea*, 1> areas = {&items};
    IssueTable table;
    table.SetIssues({Issue(ItemDataIssueSeverity::Fatal, "f1"), Issue(ItemDataIssueSeverity::Error, "e1")});
    RenderFrame(table, areas);

    const ImGuiWindow* window = ImGui::FindWindowByName(WindowName);
    REQUIRE(window != nullptr);
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed("###ShowFatal", nullptr, window->ID));
    RenderFrame(table, areas);

    // The box hides the rows, not the count.
    CHECK(table.GetCount(ItemDataIssueSeverity::Fatal) == 1);
    ClickRow(table, areas, 0, I18N::Editor::Fatal);
    CHECK(items.opened.empty());
    ClickRow(table, areas, 1, I18N::Editor::Error);
    CHECK(items.opened == "e1");
}

TEST_CASE("Data Checks' table hides an area's problems, and the counts follow [editor]")
{
    TestImGuiContext context;
    RecordingArea items("Items");
    RecordingArea monsters("Monsters");
    const std::array<DataCheckArea*, 2> areas = {&items, &monsters};
    IssueTable table;
    table.SetIssues({Issue(ItemDataIssueSeverity::Fatal, "item fatal", 0),
                     Issue(ItemDataIssueSeverity::Fatal, "monster fatal", 1),
                     Issue(ItemDataIssueSeverity::Warning, "monster warning", 1)});

    table.SetAreaShown(1, false);

    CHECK_FALSE(table.IsAreaShown(1));
    CHECK(table.IsAreaShown(0));
    CHECK(table.GetCount(ItemDataIssueSeverity::Fatal) == 1);
    CHECK(table.GetCount(ItemDataIssueSeverity::Warning) == 0);
    RenderFrame(table, areas);
    // The monsters' fatal problem (index 1) has no row; the items' (index 0) has.
    ClickRow(table, areas, 1, I18N::Editor::Fatal);
    CHECK(monsters.opened.empty());
    ClickRow(table, areas, 0, I18N::Editor::Fatal);
    CHECK(items.opened == "item fatal");

    table.SetAreaShown(1, true);
    CHECK(table.GetCount(ItemDataIssueSeverity::Fatal) == 2);
    CHECK(table.GetCount(ItemDataIssueSeverity::Warning) == 1);
}

#endif // _EDITOR
