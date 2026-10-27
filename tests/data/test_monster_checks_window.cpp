#include "stdafx.h"

#include "doctest.h"

// The Monster Checks window driven through ImGui (MN1): a click on a problem
// selects its monster and opens the tool of its field, also for a monster
// removed since the check.
#ifdef _EDITOR

#include "TestImGuiContext.h"
#include "ToolWindowsScope.h"

#include "Core/MuEditorCore.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "I18N/All.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/MonsterTools/MonsterEdits.h"
#include "UI/MonsterTools/MonsterSelection.h"
#include "UI/MonsterTools/MuMonsterChecksUI.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <string>
#include <vector>

using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;

namespace
{
constexpr char TableId[] = "##Issues";
constexpr ImVec2 WindowSize(1000.0f, 600.0f);

MonsterDefinition Monster(int number, const char* english, MonsterKind kind = MonsterKind::Monster)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", english);
    monster.kind = kind;
    return monster;
}

// The monster table with these monsters, no models and an empty history.
void StartWith(const std::vector<MonsterDefinition>& monsters)
{
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterModelDatabase.Build({});
    g_MonsterEdits.OnDataReplaced();
}

void RenderChecks()
{
    bool open = true;
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(WindowSize);
    g_MuMonsterChecksUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

const ImGuiWindow* ChecksWindow()
{
    const ImGuiWindow* window = ImGui::FindWindowByName(MuEditor::WindowIds::MonsterChecks);
    REQUIRE(window != nullptr);
    return window;
}

// As a click: pressed in the next frame.
void Click(ImGuiID id)
{
    ImGui::ActivateItemByID(id);
    RenderChecks();
}

// The checks run again on the table as it is now. The window keeps the
// problems of its last check across tests, so each test checks first.
void PressCheck()
{
    Click(ImGui::GetIDWithSeed(I18N::Editor::Check, nullptr, ChecksWindow()->ID));
}

// The row of a problem: its index among the problems, errors first, and its
// kind, which is the row's label.
ImGuiID ProblemRowId(int index, const char* kindLabel)
{
    const ImGuiID table = ImGui::GetIDWithSeed(TableId, nullptr, ChecksWindow()->ID);
    return ImGui::GetIDWithSeed(kindLabel, nullptr, ImGui::GetIDWithSeed(index, table));
}
} // namespace

TEST_CASE("Monster Checks: a click on a name problem selects the monster and opens Monster Translations "
          "[editor][monsters]")
{
    const ToolWindowsScope windows;
    StartWith({Monster(2, "Budge Dragon"), Monster(252, "", MonsterKind::Npc), Monster(7, "Budge||Dragon")});
    g_MonsterSelection.Select(2);
    TestImGuiContext context;
    RenderChecks();
    PressCheck();

    // The separator of 7 is the only error, so it comes first.
    Click(ProblemRowId(0, I18N::Editor::Error));

    CHECK(g_MonsterSelection.GetNumber() == 7);
    CHECK(g_MuEditorCore.GetToolWindows().monsterTranslations);
    CHECK_FALSE(g_MuEditorCore.GetToolWindows().monsterBrowser);

    Click(ProblemRowId(1, I18N::Editor::Warning));

    CHECK(g_MonsterSelection.GetNumber() == 252);
}

TEST_CASE("Monster Checks: the problems stay until Check, also of a monster removed since [editor][monsters]")
{
    const ToolWindowsScope windows;
    // A name with a format and an ImGui id, in a language the game has no
    // texts for.
    MonsterDefinition lich = Monster(9, "100%s ##Lich");
    lich.names.Set("zz", "Lich");
    StartWith({Monster(2, "Budge Dragon"), lich});
    TestImGuiContext context;
    RenderChecks();
    PressCheck();

    g_MonsterDatabase.Remove(9);
    RenderChecks();
    Click(ProblemRowId(0, I18N::Editor::Warning));

    CHECK(g_MonsterSelection.GetNumber() == 9);
    CHECK(g_MuEditorCore.GetToolWindows().monsterTranslations);

    PressCheck();
    // No problem is left, so there is no row to click.
    g_MonsterSelection.Select(2);
    Click(ProblemRowId(0, I18N::Editor::Warning));
    CHECK(g_MonsterSelection.GetNumber() == 2);
}

#endif // _EDITOR
