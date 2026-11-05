#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "TestImGuiContext.h"

#include "Data/GameData/Common/LocalizedString.h"
#include "I18N/All.h"
#include "UI/Common/NamesGrid.h"
#include "UI/Common/NamesGridRows.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace
{
constexpr char GridWindow[] = "Names###NamesGridTest";
constexpr char GridTable[] = "##Names";

struct Commit
{
    int id;
    std::string locale;
    std::string text;
    bool operator==(const Commit&) const = default;
};

// Two rows, 2 and 9, and the commits the grid asked for.
class FakeRows final : public MuEditor::NamesGridRows
{
public:
    FakeRows()
    {
        m_names[2].Set("en", "Budge Dragon");
        m_names[2].Set("pt", "Dragão Lento");
        m_names[9].Set("en", "Thunder Lich");
    }

    const std::vector<int>& List(std::string_view search, std::string_view missingLocale) override
    {
        m_listed.clear();
        for (const auto& [id, names] : m_names)
        {
            const bool found = search.empty() || names.GetNeutral().find(search) != std::string::npos;
            const bool missing = missingLocale.empty() || !names.HasOwn(missingLocale);
            if (found && missing)
            {
                m_listed.push_back(id);
            }
        }
        return m_listed;
    }
    void Refresh() override {}
    int GetVersion() const override
    {
        return 1;
    }
    std::vector<int> CountMissing(std::span<const char* const> locales) const override
    {
        return std::vector<int>(locales.size(), 0);
    }
    const Data::LocalizedString* FindNames(int id) const override
    {
        const auto found = m_names.find(id);
        return found != m_names.end() ? &found->second : nullptr;
    }
    void FormatId(int id, char* buffer, std::size_t size) const override
    {
        std::snprintf(buffer, size, "%d", id);
    }
    void CommitName(int id, const char* locale, const char* text) override
    {
        commits.push_back({id, locale, text});
        m_names[id].Set(locale, text);
    }
    int GetSelected() const override
    {
        return selected;
    }
    int GetSelectionSerial() const override
    {
        return serial;
    }
    void Select(int id) override
    {
        selected = id;
        ++serial;
    }
    void ShowBrowser() override {}
    const char* GetSelectTooltip() const override
    {
        return "";
    }
    const char* GetAllRowsLabel() const override
    {
        return "All";
    }

    std::vector<Commit> commits;
    int selected = -1;
    int serial = 0;

private:
    std::map<int, Data::LocalizedString> m_names;
    std::vector<int> m_listed;
};

// One frame of the grid's window; `draw` false leaves the window out, as F12
// hides the editor.
void RenderGrid(MuEditor::NamesGrid& grid, FakeRows& rows, bool draw = true, const char* typed = nullptr)
{
    if (typed != nullptr)
    {
        ImGui::GetIO().AddInputCharactersUTF8(typed);
    }
    ImGui::NewFrame();
    grid.BeginFrame();
    if (draw)
    {
        ImGui::Begin(GridWindow);
        grid.UpdateCountsAndLabels(rows);
        grid.RenderToolbar(rows);
        grid.RenderTable(rows, GridTable);
        ImGui::End();
    }
    grid.CommitEditNotDrawn(rows);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

// The name field of a row in a language's column (1 is the first language).
ImGuiID NameCellId(int id, int column)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(GridWindow);
    REQUIRE(window != nullptr);
    const ImGuiID table = ImGui::GetIDWithSeed(GridTable, nullptr, window->ID);
    const ImGuiID row = ImGui::GetIDWithSeed(id, table);
    return ImGui::GetIDWithSeed("##Name", nullptr, ImGui::GetIDWithSeed(column, row));
}

int ColumnOf(const char* locale)
{
    const auto locales = I18N::GetAvailableLocales();
    for (size_t index = 0; index < locales.size(); ++index)
    {
        if (std::string_view(locales[index]) == locale)
        {
            return static_cast<int>(index);
        }
    }
    FAIL("no such UI language");
    return -1;
}
} // namespace

TEST_CASE("Names grid: a name is stored when its field is left [editor][names-grid]")
{
    TestImGuiContext context;
    MuEditor::NamesGrid grid;
    FakeRows rows;
    RenderGrid(grid, rows);
    RenderGrid(grid, rows);

    // Activation selects the whole text; the typed text replaces it.
    ImGui::ActivateItemByID(NameCellId(9, ColumnOf("pt")));
    RenderGrid(grid, rows);
    RenderGrid(grid, rows, true, "Lich do Trovão");
    CHECK(rows.commits.empty());
    ImGui::ClearActiveID();
    RenderGrid(grid, rows);

    CHECK(rows.commits == std::vector<Commit>{{9, "pt", "Lich do Trovão"}});
}

TEST_CASE("Names grid: an edit is stored when the grid stops being drawn [editor][names-grid]")
{
    TestImGuiContext context;
    MuEditor::NamesGrid grid;
    FakeRows rows;
    RenderGrid(grid, rows);
    RenderGrid(grid, rows);

    ImGui::ActivateItemByID(NameCellId(2, ColumnOf("en")));
    RenderGrid(grid, rows);
    RenderGrid(grid, rows, true, "Budge");
    RenderGrid(grid, rows, false);

    CHECK(rows.commits == std::vector<Commit>{{2, "en", "Budge"}});
}

TEST_CASE("Names grid: an unchanged name is not stored [editor][names-grid]")
{
    TestImGuiContext context;
    MuEditor::NamesGrid grid;
    FakeRows rows;
    RenderGrid(grid, rows);
    RenderGrid(grid, rows);

    ImGui::ActivateItemByID(NameCellId(2, ColumnOf("pt")));
    RenderGrid(grid, rows);
    RenderGrid(grid, rows, true, "Dragão Lento");
    ImGui::ClearActiveID();
    RenderGrid(grid, rows);

    CHECK(rows.commits.empty());
}

#endif // _EDITOR
