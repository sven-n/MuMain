#include "stdafx.h"

#include "doctest.h"

// The Data Checks window driven through ImGui: a click on a problem of a
// monster selects the monster and opens the tool of its field, also for a
// monster removed since the check; an area's box hides its problems.
#ifdef _EDITOR

#include "TestImGuiContext.h"
#include "ToolWindowsScope.h"

#include "Core/MuEditorCore.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "I18N/All.h"
#include "Render/Look/LookTuning.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/Common/SavedDataFiles.h"
#include "UI/DataChecks/MuDataChecksUI.h"
#include "UI/MonsterTools/MonsterCheckArea.h"
#include "UI/MonsterTools/MonsterEdits.h"
#include "UI/MonsterTools/MonsterSelection.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <vector>

using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;
using MuEditor::DataChecks::MuDataChecksUI;

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

// A Data Checks window with the monsters as its only area.
std::unique_ptr<MuDataChecksUI> MonsterChecksWindow()
{
    std::vector<std::unique_ptr<MuEditor::DataChecks::DataCheckArea>> areas;
    areas.push_back(MuEditor::Monsters::CreateMonsterCheckArea());
    return std::make_unique<MuDataChecksUI>(std::move(areas));
}

void RenderChecks(MuDataChecksUI& checks)
{
    bool open = true;
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(WindowSize);
    checks.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

const ImGuiWindow* ChecksWindow()
{
    const ImGuiWindow* window = ImGui::FindWindowByName(MuEditor::WindowIds::DataChecks);
    REQUIRE(window != nullptr);
    return window;
}

// As a click: pressed in the next frame.
void Click(MuDataChecksUI& checks, ImGuiID id)
{
    ImGui::ActivateItemByID(id);
    RenderChecks(checks);
}

void PressCheckAll(MuDataChecksUI& checks)
{
    Click(checks, ImGui::GetIDWithSeed(I18N::Editor::CheckAllData, nullptr, ChecksWindow()->ID));
}

// The box of the area at `index`, labelled with its name.
ImGuiID AreaBoxId(int index, const char* name)
{
    return ImGui::GetIDWithSeed(name, nullptr, ImGui::GetIDWithSeed(index, ChecksWindow()->ID));
}

// The row of a problem: its index among the problems, the worst first, and its
// level, which is the row's label.
ImGuiID ProblemRowId(int index, const char* levelLabel)
{
    const ImGuiID table = ImGui::GetIDWithSeed(TableId, nullptr, ChecksWindow()->ID);
    return ImGui::GetIDWithSeed(levelLabel, nullptr, ImGui::GetIDWithSeed(index, table));
}
} // namespace

TEST_CASE("Data Checks: a click on a monster's name problem selects the monster and opens Monster Translations "
          "[editor][monsters]")
{
    const ToolWindowsScope windows;
    StartWith({Monster(2, "Budge Dragon"), Monster(252, "", MonsterKind::Npc), Monster(7, "Budge||Dragon")});
    g_MonsterSelection.Select(2);
    TestImGuiContext context;
    const std::unique_ptr<MuDataChecksUI> checks = MonsterChecksWindow();
    RenderChecks(*checks);

    // The separator of 7 is the only fatal problem, so it comes first.
    Click(*checks, ProblemRowId(0, I18N::Editor::Fatal));

    CHECK(g_MonsterSelection.GetNumber() == 7);
    CHECK(g_MuEditorCore.GetToolWindows().monsterTranslations);
    CHECK_FALSE(g_MuEditorCore.GetToolWindows().monsterBrowser);

    Click(*checks, ProblemRowId(1, I18N::Editor::Warning));

    CHECK(g_MonsterSelection.GetNumber() == 252);
}

TEST_CASE("Data Checks: the problems stay until Check all, also of a monster removed since [editor][monsters]")
{
    const ToolWindowsScope windows;
    // A name with a format and an ImGui id, in a language the game has no
    // texts for.
    MonsterDefinition lich = Monster(9, "100%s ##Lich");
    lich.names.Set("zz", "Lich");
    StartWith({Monster(2, "Budge Dragon"), lich});
    TestImGuiContext context;
    const std::unique_ptr<MuDataChecksUI> checks = MonsterChecksWindow();
    RenderChecks(*checks);

    g_MonsterDatabase.Remove(9);
    RenderChecks(*checks);
    Click(*checks, ProblemRowId(0, I18N::Editor::Warning));

    CHECK(g_MonsterSelection.GetNumber() == 9);
    CHECK(g_MuEditorCore.GetToolWindows().monsterTranslations);

    PressCheckAll(*checks);
    // No problem is left, so there is no row to click.
    g_MonsterSelection.Select(2);
    Click(*checks, ProblemRowId(0, I18N::Editor::Warning));
    CHECK(g_MonsterSelection.GetNumber() == 2);
}

TEST_CASE("Data Checks: a save of the server list counts as a change since the check [editor]")
{
    const ToolWindowsScope windows;
    StartWith({Monster(2, "Budge Dragon")});
    TestImGuiContext context;
    const std::unique_ptr<MuDataChecksUI> checks = MonsterChecksWindow();
    RenderChecks(*checks);
    CHECK_FALSE(checks->DataChangedSinceCheck());

    MuEditor::SavedDataFiles::NoteSaved();
    CHECK(checks->DataChangedSinceCheck());

    PressCheckAll(*checks);
    CHECK_FALSE(checks->DataChangedSinceCheck());
}

// An empty client folder with the look files' folders; the working directory
// while it lives, so a look save writes its files there.
class LookFolder
{
public:
    LookFolder()
        // CTest runs the test cases as parallel processes; each needs its own folder.
        : m_directory(std::filesystem::temp_directory_path() /
                      ("mu_test_data_checks_looks_" + std::to_string(std::random_device{}()))),
          m_previousDirectory(std::filesystem::current_path())
    {
        // Set up the log file before switching directories, so none is written there.
        mu::log::Init();
        std::filesystem::create_directories(m_directory / "Data" / "Render");
        std::filesystem::create_directories(m_directory / "Data" / "Maps");
        std::filesystem::current_path(m_directory);
    }

    ~LookFolder()
    {
        std::error_code ignored;
        std::filesystem::current_path(m_previousDirectory, ignored);
        std::filesystem::remove_all(m_directory, ignored);
    }

    LookFolder(const LookFolder&) = delete;
    LookFolder& operator=(const LookFolder&) = delete;

private:
    std::filesystem::path m_directory;
    std::filesystem::path m_previousDirectory;
};

TEST_CASE("Data Checks: a save of the looks, also by $look save, counts as a change since the check [editor]")
{
    const ToolWindowsScope windows;
    StartWith({Monster(2, "Budge Dragon")});
    TestImGuiContext context;
    const std::unique_ptr<MuDataChecksUI> checks = MonsterChecksWindow();
    RenderChecks(*checks);
    CHECK_FALSE(checks->DataChangedSinceCheck());

    // The chat command saves without the DevEditor; both go through SaveLive.
    const LookFolder folder;
    REQUIRE(Render::Look::HandleDeveloperCommand(L"$look save"));
    CHECK(checks->DataChangedSinceCheck());

    PressCheckAll(*checks);
    CHECK_FALSE(checks->DataChangedSinceCheck());
}

TEST_CASE("Data Checks: an area's box hides its problems [editor][monsters]")
{
    const ToolWindowsScope windows;
    StartWith({Monster(2, "Budge Dragon"), Monster(7, "Budge||Dragon")});
    g_MonsterSelection.Select(2);
    TestImGuiContext context;
    const std::unique_ptr<MuDataChecksUI> checks = MonsterChecksWindow();
    RenderChecks(*checks);

    Click(*checks, AreaBoxId(0, I18N::Editor::Monsters));
    Click(*checks, ProblemRowId(0, I18N::Editor::Fatal));
    CHECK(g_MonsterSelection.GetNumber() == 2);

    Click(*checks, AreaBoxId(0, I18N::Editor::Monsters));
    Click(*checks, ProblemRowId(0, I18N::Editor::Fatal));
    CHECK(g_MonsterSelection.GetNumber() == 7);
}

TEST_CASE("Data Checks: the editor's window with all its areas draws without ImGui errors [editor]")
{
    const ToolWindowsScope windows;
    StartWith({Monster(2, "Budge Dragon")});
    TestImGuiContext context;
    constexpr int Frames = 3;
    for (int frame = 0; frame < Frames; ++frame)
    {
        RenderChecks(g_MuDataChecksUI);
    }
    CHECK(ImGui::FindWindowByName(MuEditor::WindowIds::DataChecks) != nullptr);
}

#endif // _EDITOR
