#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "TestImGuiContext.h"
#include "ToolWindowsScope.h"

#include "Core/MuEditorCore.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "UI/Common/MuEditorLayout.h"
#include "UI/Common/MuEditorToolWindows.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/Common/ToolGroups.h"
#include "UI/Common/UndoTarget.h"
#include "UI/MonsterTools/MonsterEdits.h"
#include "UI/MonsterTools/MonsterSelection.h"
#include "UI/MonsterTools/MonsterToolWindow.h"
#include "UI/MonsterTools/MuMonsterBrowserUI.h"
#include "UI/MonsterTools/MuMonsterChecksUI.h"
#include "UI/MonsterTools/MuMonsterTranslationsUI.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <array>

using Data::Monsters::MonsterDefinition;
using MuEditor::Monsters::MonsterSelection;
namespace WindowIds = MuEditor::WindowIds;

namespace
{
constexpr ImVec2 EditorWindowSize(1200.0f, 800.0f);

MonsterDefinition Monster(int number, const char* english)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", english);
    return monster;
}

void RenderMonsterTools()
{
    bool open = true;
    g_MuMonsterBrowserUI.Render(&open);
    g_MuMonsterTranslationsUI.Render(&open);
    g_MuMonsterChecksUI.Render(&open);
}

// The editor window, which holds the dock space of the tool windows.
void BeginEditorWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(EditorWindowSize);
    ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoDocking);
}

ImGuiID DockIdOf(const char* windowId)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(windowId);
    REQUIRE(window != nullptr);
    return window->DockId;
}
} // namespace

// Draws the monster tools for a few frames, with a monster, a name that holds
// a format and an ImGui id, a number without an entry and no selection: ImGui
// counts what the windows get wrong (Begin and End out of step) as errors.
TEST_CASE("The monster tools draw their frames without ImGui errors [editor][monsters]")
{
    const std::array monsters = {Monster(2, "Budge Dragon"), Monster(9, "100%s ##Lich")};
    g_MonsterDatabase.Build(monsters, "en");
    TestImGuiContext context;

    constexpr int Frames = 5;
    for (const int number : {2, 9, 76, MonsterSelection::NoMonster})
    {
        g_MonsterSelection.Select(number);
        for (int frame = 0; frame < Frames; ++frame)
        {
            ImGui::NewFrame();
            RenderMonsterTools();
            ImGui::Render();
            INFO("monster " << number << ", frame " << frame);
            CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
        }
    }
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterBrowser) != nullptr);
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterTranslations) != nullptr);
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterChecks) != nullptr);
}

TEST_CASE("The first layout docks the monster browser beside the item browser, its tools with the others "
          "[editor][monsters]")
{
    TestImGuiContext context;
    const auto frame = []
    {
        ImGui::NewFrame();
        BeginEditorWindow();
        MuEditor::Layout::SubmitDockSpace(false);
        ImGui::End();
        for (const char* window : {WindowIds::ItemBrowser, WindowIds::ItemBasics, WindowIds::MonsterBrowser,
                                   WindowIds::MonsterTranslations, WindowIds::MonsterChecks})
        {
            ImGui::Begin(window);
            ImGui::End();
        }
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    frame();
    frame();

    CHECK(DockIdOf(WindowIds::MonsterBrowser) == DockIdOf(WindowIds::ItemBrowser));
    CHECK(DockIdOf(WindowIds::MonsterTranslations) == DockIdOf(WindowIds::ItemBasics));
    CHECK(DockIdOf(WindowIds::MonsterChecks) == DockIdOf(WindowIds::ItemBasics));
    CHECK(DockIdOf(WindowIds::MonsterBrowser) != DockIdOf(WindowIds::ItemBasics));
}

// A layout saved before the monster tools: the item browser beside the item
// tools above the console. A monster window opened the first time docks with
// the item windows: the browser with the item browser also after a monster
// tool found its place.
TEST_CASE("A monster window opened the first time in a layout from before the monster tools docks with the item "
          "windows [editor][monsters]")
{
    TestImGuiContext context;
    ImGuiID browser = 0;
    ImGuiID tools = 0;
    const auto frame = [&]
    {
        ImGui::NewFrame();
        BeginEditorWindow();
        const ImGuiID dockSpace = ImGui::GetID("ToolWindows");
        if (ImGui::DockBuilderGetNode(dockSpace) == nullptr)
        {
            ImGui::DockBuilderAddNode(dockSpace, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockSpace, ImVec2(EditorWindowSize.x, EditorWindowSize.y - 20.0f));
            ImGuiID above = 0;
            const ImGuiID console = ImGui::DockBuilderSplitNode(dockSpace, ImGuiDir_Down, 0.25f, nullptr, &above);
            browser = ImGui::DockBuilderSplitNode(above, ImGuiDir_Left, 0.35f, nullptr, &tools);
            ImGui::DockBuilderDockWindow(WindowIds::Console, console);
            ImGui::DockBuilderDockWindow(WindowIds::ItemBrowser, browser);
            ImGui::DockBuilderDockWindow(WindowIds::ItemBasics, tools);
            ImGui::DockBuilderFinish(dockSpace);
        }
        // The node is there: it is not built again.
        MuEditor::Layout::SubmitDockSpace(false);
        ImGui::End();
        for (const char* window : {WindowIds::ItemBrowser, WindowIds::ItemBasics})
        {
            ImGui::Begin(window);
            ImGui::End();
        }
        for (const char* window : {WindowIds::MonsterTranslations, WindowIds::MonsterBrowser})
        {
            MuEditor::Layout::DockInGroup(window, MuEditor::GetMonsterTools());
            ImGui::Begin(window);
            ImGui::End();
        }
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    frame();
    frame();

    CHECK(DockIdOf(WindowIds::MonsterBrowser) == browser);
    CHECK(DockIdOf(WindowIds::MonsterTranslations) == tools);
}

TEST_CASE("The editor bar's undo is the monster tools' while a monster tool is open [editor][monsters]")
{
    ToolWindowsScope windows;
    MuEditor::UndoTarget& target = MuEditor::Monsters::ToolWindow::GetUndoTarget();
    const std::array monsters = {Monster(2, "Budge Dragon")};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    REQUIRE(g_MonsterEdits.SetMonsterName("Name", 2, "en", "Budge Dragon 2"));

    CHECK_FALSE(target.IsOpen());
    g_MuEditorCore.GetToolWindows().monsterChecks = true;
    CHECK(target.IsOpen());
    CHECK(target.CanUndo());
    CHECK(target.GetUndoLabel() == "Name");

    // Changed past the history: the undo is refused (and logged).
    g_MonsterDatabase.Set(Monster(2, "Budge Dragon 3"));
    target.Undo();
    CHECK(g_MonsterDatabase.Find(2)->names.GetNeutral() == "Budge Dragon 3");
    CHECK(target.CanUndo());
}

#endif // _EDITOR
