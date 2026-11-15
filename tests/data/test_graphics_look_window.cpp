#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "TestImGuiContext.h"

#include "Data/GameData/MapData/LookProfile.h"
#include "I18N/All.h"
#include "Render/Look/LookTuning.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/GraphicsLook/MuGraphicsLookUI.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <system_error>
#include <vector>

namespace
{
using Data::Maps::LookValue;

// Blood Castle 1 and 2 share an entry that uses the environment "dark"; "dim" and "pale" are unused.
constexpr const char* LookText = R"({"formatVersion": 2,
  "default": {"bloom.strength": 1, "mood.base": 0.7, "mood.figures": 0.7},
  "environments": {"dark": {"bloom.strength": 0.5}, "dim": {}, "pale": {}},
  "maps": [{"maps": [11, 12], "name": "Blood Castle", "environment": "dark"}]})";

// A client folder with that Data/Render/Look.json, live and the working
// directory while it lives.
class LookFolder
{
public:
    LookFolder()
        // CTest runs the test cases as parallel processes; each needs its own folder.
        : m_directory(std::filesystem::temp_directory_path() /
                      ("mu_test_graphics_look_window_" + std::to_string(std::random_device{}()))),
          m_previousDirectory(std::filesystem::current_path())
    {
        std::filesystem::create_directories(m_directory / "Data" / "Render");
        std::ofstream(m_directory / Data::Maps::LookProfileFile, std::ios::binary) << LookText;
        std::filesystem::current_path(m_directory);
        REQUIRE(Render::Look::LoadLive({}));
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

const Data::Maps::LookProfileData& Live()
{
    return Render::Look::LiveFiles().lookData;
}

void RenderWindow()
{
    bool open = true;
    ImGui::NewFrame();
    g_MuGraphicsLookUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

// One of the window's panes: "##LookScopes" or "##LookScope". ImGui names a
// child window "<parent>/<name>_<id>".
ImGuiID PaneId(const char* pane)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(MuEditor::WindowIds::GraphicsLook);
    REQUIRE(window != nullptr);
    const std::string infix = std::string("/") + pane + "_";
    for (const ImGuiWindow* child : ImGui::GetCurrentContext()->Windows)
    {
        if (child->ParentWindow == window && std::strstr(child->Name, infix.c_str()) != nullptr)
        {
            return child->ID;
        }
    }
    FAIL("the window has no pane " << pane);
    return 0;
}

ImGuiID WidgetId(const char* pane, const std::string& label)
{
    return ImGui::GetIDWithSeed(label.c_str(), nullptr, PaneId(pane));
}

// The "Set here" box of `value` in the selected scope's rows.
ImGuiID SetHereId(LookValue value)
{
    return ImGui::GetIDWithSeed("##SetHere", nullptr,
                                ImGui::GetIDWithSeed(static_cast<int>(value), PaneId("##LookScope")));
}

std::string EnvironmentLabel(const char* name, int users, int index)
{
    return std::string(name) + "  (" + I18N::Format(I18N::Editor::LookUsedBy0Maps, {std::to_string(users)}) + ")##Env" +
           std::to_string(index);
}

void Press(ImGuiID id)
{
    ImGui::ActivateItemByID(id);
    RenderWindow();
    RenderWindow();
}

// Activation selects all of the field's text; the typed text replaces it.
void Type(ImGuiID id, const char* text)
{
    ImGui::ActivateItemByID(id);
    RenderWindow();
    ImGui::GetIO().AddInputCharactersUTF8(text);
    RenderWindow();
}

// The window on the folder's file, with the default selected and nothing to undo.
void Start()
{
    RenderWindow();
    Press(WidgetId("##LookScopes", I18N::Editor::LookScopeDefault));
    REQUIRE_FALSE(g_MuGraphicsLookUI.CanUndo());
}
} // namespace

TEST_CASE("Graphics Look window: a value set on the default goes live as one undo step [editor][look]")
{
    I18N::SetLocale("en");
    LookFolder folder;
    TestImGuiContext context;
    Start();

    REQUIRE_FALSE(Live().defaultOverrides[LookValue::GradingStrength].has_value());
    Press(SetHereId(LookValue::GradingStrength));
    REQUIRE(Live().defaultOverrides[LookValue::GradingStrength].has_value());
    CHECK(g_MuGraphicsLookUI.GetUndoLabel().find("grading.strength") != std::string::npos);

    g_MuGraphicsLookUI.Undo();
    RenderWindow();
    CHECK_FALSE(Live().defaultOverrides[LookValue::GradingStrength].has_value());
    CHECK(g_MuGraphicsLookUI.CanRedo());

    // A box cleared again takes the value from below: the default's own bloom goes.
    Press(SetHereId(LookValue::BloomStrength));
    CHECK_FALSE(Live().defaultOverrides[LookValue::BloomStrength].has_value());
}

TEST_CASE("Graphics Look window: a scope several maps use stays locked until it is confirmed [editor][look]")
{
    I18N::SetLocale("en");
    LookFolder folder;
    TestImGuiContext context;
    Start();

    Press(WidgetId("##LookScopes", EnvironmentLabel("dark", 2, 0)));
    Press(SetHereId(LookValue::MoodSun));
    CHECK_FALSE(Live().environments[0].overrides[LookValue::MoodSun].has_value());

    Press(WidgetId("##LookScope", I18N::Editor::LookEditForAll));
    Press(SetHereId(LookValue::MoodSun));
    CHECK(Live().environments[0].overrides[LookValue::MoodSun].has_value());
}

TEST_CASE("Graphics Look window: an environment is added, renamed and removed, and undone [editor][look]")
{
    I18N::SetLocale("en");
    LookFolder folder;
    TestImGuiContext context;
    Start();

    Type(WidgetId("##LookScopes", "##NewEnvironment"), "caves");
    Press(WidgetId("##LookScopes", I18N::Editor::Add));
    REQUIRE(Live().environments.size() == 4);
    CHECK(Live().environments[3].name == "caves");

    // The new environment is selected; nothing uses it, so it needs no confirmation.
    Type(WidgetId("##LookScope", "##EnvironmentName"), "cold");
    Press(WidgetId("##LookScope", I18N::Editor::Rename));
    CHECK(Live().environments[3].name == "cold");

    Press(WidgetId("##LookScope", I18N::Editor::Remove));
    CHECK(Live().environments.size() == 3);

    g_MuGraphicsLookUI.Undo();
    g_MuGraphicsLookUI.Undo();
    RenderWindow();
    REQUIRE(Live().environments.size() == 4);
    CHECK(Live().environments[3].name == "caves");
}

TEST_CASE("Graphics Look window: a $look change is one undo step, and Revert starts a fresh history [editor][look]")
{
    I18N::SetLocale("en");
    LookFolder folder;
    TestImGuiContext context;
    Start();

    const Render::Look::LookEdit edit{"bloom.strength", 0.25};
    REQUIRE(Render::Look::SetLive({&edit, 1}, false));
    RenderWindow();
    CHECK(g_MuGraphicsLookUI.GetUndoLabel() == "$look");
    g_MuGraphicsLookUI.Undo();
    RenderWindow();
    CHECK(Live().defaultOverrides[LookValue::BloomStrength] == 1.0);

    Press(SetHereId(LookValue::GradingStrength));
    REQUIRE(g_MuGraphicsLookUI.CanUndo());
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed(I18N::Editor::Revert, nullptr,
                                                 ImGui::FindWindowByName(MuEditor::WindowIds::GraphicsLook)->ID));
    RenderWindow();
    RenderWindow();
    CHECK_FALSE(g_MuGraphicsLookUI.CanUndo());
    CHECK_FALSE(Live().defaultOverrides[LookValue::GradingStrength].has_value());
}

TEST_CASE("Graphics Look window: removing an environment in the middle selects the default [editor][look]")
{
    I18N::SetLocale("en");
    LookFolder folder;
    TestImGuiContext context;
    Start();

    Press(WidgetId("##LookScopes", EnvironmentLabel("dim", 0, 1)));
    Press(WidgetId("##LookScope", I18N::Editor::Remove));
    REQUIRE(Live().environments.size() == 2);
    CHECK(Live().environments[1].name == "pale");

    // The default is selected: the rename box of "pale" never shows "dim".
    Press(SetHereId(LookValue::GradingStrength));
    CHECK(Live().defaultOverrides[LookValue::GradingStrength].has_value());
    CHECK(Live().environments[1].name == "pale");
}

TEST_CASE("Graphics Look window: an undo while the window is hidden takes back a $look change first [editor][look]")
{
    I18N::SetLocale("en");
    LookFolder folder;
    TestImGuiContext context;
    Start();
    Press(SetHereId(LookValue::GradingStrength));

    // The window isn't drawn again before the editor bar's Undo.
    const Render::Look::LookEdit edit{"bloom.strength", 0.25};
    REQUIRE(Render::Look::SetLive({&edit, 1}, false));
    g_MuGraphicsLookUI.Undo();
    CHECK(Live().defaultOverrides[LookValue::BloomStrength] == 1.0);
    CHECK(Live().defaultOverrides[LookValue::GradingStrength].has_value());
    CHECK(g_MuGraphicsLookUI.GetRedoLabel() == "$look");
}

TEST_CASE("Graphics Look window: a value set on a shared map entry goes to the entry [editor][look]")
{
    I18N::SetLocale("en");
    LookFolder folder;
    TestImGuiContext context;
    Start();

    Press(WidgetId("##LookScopes", "Blood Castle  [11, 12]##Entry11"));
    Press(WidgetId("##LookScope", I18N::Editor::LookEditForAll));
    Press(SetHereId(LookValue::GradingStrength));
    const Data::Maps::MapLookOverrides* entry = Live().FindMap(12);
    REQUIRE(entry != nullptr);
    CHECK(entry->overrides[LookValue::GradingStrength].has_value());
    CHECK(entry->maps == std::vector<int>{11, 12});
}

#endif // _EDITOR
