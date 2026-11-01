#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "TestImGuiContext.h"

#include "Data/GameData/ServerListData/ServerListJson.h"
#include "I18N/All.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/ServerList/MuServerListUI.h"
#include "UI/ServerList/ServerListColors.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <random>
#include <string>
#include <system_error>
#include <vector>

namespace
{
using Data::ServerList::ServerGroup;

// A client folder whose Data/Local/ServerList.json has Valhalla, Helheim and
// Midgard; the working directory while it lives, as the window reads the file
// from there.
class ServerListFolder
{
public:
    ServerListFolder()
        // CTest runs the test cases as parallel processes; each needs its own folder.
        : m_directory(std::filesystem::temp_directory_path() /
                      ("mu_test_server_list_window_" + std::to_string(std::random_device{}()))),
          m_previousDirectory(std::filesystem::current_path())
    {
        Data::ServerList::ServerListData list;
        int id = 0;
        for (const char* name : {"Valhalla", "Helheim", "Midgard"})
        {
            ServerGroup group;
            group.id = id++;
            group.name = name;
            list.groups.push_back(group);
        }
        std::vector<Data::Items::ItemDataIssue> issues;
        REQUIRE(Data::ServerList::SaveServerListFile(m_directory / Data::ServerList::ServerListFile, list, issues));
        std::filesystem::current_path(m_directory);
    }

    ~ServerListFolder()
    {
        std::error_code ignored;
        std::filesystem::current_path(m_previousDirectory, ignored);
        std::filesystem::remove_all(m_directory, ignored);
    }

    ServerListFolder(const ServerListFolder&) = delete;
    ServerListFolder& operator=(const ServerListFolder&) = delete;

private:
    std::filesystem::path m_directory;
    std::filesystem::path m_previousDirectory;
};

void RenderServerList()
{
    bool open = true;
    ImGui::NewFrame();
    g_MuServerListUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

const ImGuiWindow* ServerListWindow()
{
    const ImGuiWindow* window = ImGui::FindWindowByName(MuEditor::WindowIds::ServerList);
    REQUIRE(window != nullptr);
    return window;
}

// The id of a widget in the window's toolbar.
ImGuiID ToolbarId(const char* label)
{
    return ImGui::GetIDWithSeed(label, nullptr, ServerListWindow()->ID);
}

// A tab of the window ("###ChannelNames"), in its tab bar.
ImGuiID TabId(const char* tab)
{
    const ImGuiID tabBar = ImGui::GetIDWithSeed("##ServerListTabs", nullptr, ServerListWindow()->ID);
    return ImGui::GetIDWithSeed(tab, nullptr, tabBar);
}

// One of the window's panes: "##Groups" (the list and its buttons), "##Group"
// (the selected group), "##VipTypes" or "##Names". ImGui names a child window
// "<parent>/<name>_<id>".
const ImGuiWindow* Pane(const char* pane)
{
    const ImGuiWindow* window = ServerListWindow();
    const std::string infix = std::string("/") + pane + "_";
    for (const ImGuiWindow* child : ImGui::GetCurrentContext()->Windows)
    {
        if (child->ParentWindow == window && std::strstr(child->Name, infix.c_str()) != nullptr)
        {
            return child;
        }
    }
    FAIL("the window has no pane " << pane);
    return nullptr;
}

// The id of a widget in one of the panes.
ImGuiID PaneId(const char* pane, const char* label)
{
    return ImGui::GetIDWithSeed(label, nullptr, Pane(pane)->ID);
}

// The id of a row of the group list (RenderGroupList pushes its index).
ImGuiID GroupRowId(int index)
{
    return ImGui::GetIDWithSeed("##Group", nullptr, ImGui::GetIDWithSeed(index, Pane("##Groups")->ID));
}

// As a click: pressed in the next frame, as the field that was active is left.
void Press(ImGuiID id)
{
    ImGui::ActivateItemByID(id);
    RenderServerList();
}

// Activation selects all of the field's text; the typed text replaces it.
void Type(ImGuiID id, const char* text)
{
    Press(id);
    ImGui::GetIO().AddInputCharactersUTF8(text);
    RenderServerList();
}

// The window with the folder's file read again and Valhalla selected; an
// earlier test of this process may have used the window.
void StartWithValhalla()
{
    RenderServerList();
    Press(ToolbarId(I18N::Editor::Revert));
    Press(GroupRowId(0));
    REQUIRE_FALSE(g_MuServerListUI.CanUndo());
}
} // namespace

TEST_CASE("Server List window: a typed name is a step of its own before a button moves the group [editor]")
{
    I18N::SetLocale("en");
    ServerListFolder folder;
    TestImGuiContext context;
    StartWithValhalla();

    Type(PaneId("##Group", I18N::Editor::Name), "Asgard");
    // The name field is still active when Move down is pressed.
    Press(PaneId("##Groups", I18N::Editor::MoveDown));
    RenderServerList();

    CHECK(g_MuServerListUI.GetUndoLabel() == "Move Asgard");
    g_MuServerListUI.Undo();
    CHECK(g_MuServerListUI.GetUndoLabel() == "Valhalla: name");
    CHECK(g_MuServerListUI.GetRedoLabel() == "Move Asgard");
}

TEST_CASE("Server List window: a typed name is a step of its own before the test server box [editor]")
{
    I18N::SetLocale("en");
    ServerListFolder folder;
    TestImGuiContext context;
    StartWithValhalla();

    Type(PaneId("##Group", I18N::Editor::Name), "Asgard");
    Press(PaneId("##Group", I18N::Editor::TestServer));
    RenderServerList();

    CHECK(g_MuServerListUI.GetUndoLabel() == "Asgard: test server");
    g_MuServerListUI.Undo();
    CHECK(g_MuServerListUI.GetUndoLabel() == "Valhalla: name");
}

TEST_CASE("Server List window: leaving a text field for another makes what was typed one step [editor]")
{
    I18N::SetLocale("en");
    ServerListFolder folder;
    TestImGuiContext context;
    StartWithValhalla();

    Type(PaneId("##Group", I18N::Editor::Name), "Asgard");
    Type(PaneId("##Group", I18N::Editor::Description), "North");
    RenderServerList();

    CHECK(g_MuServerListUI.GetUndoLabel() == "Asgard: description");
    g_MuServerListUI.Undo();
    CHECK(g_MuServerListUI.GetUndoLabel() == "Valhalla: name");
}

TEST_CASE("Server List window: a VIP type added on the Channel names tab is a step [editor]")
{
    I18N::SetLocale("en");
    ServerListFolder folder;
    TestImGuiContext context;
    StartWithValhalla();

    // A tab picked shows from the next frame on.
    Press(TabId("###ChannelNames"));
    RenderServerList();
    Press(PaneId("##VipTypes", I18N::Editor::Add));
    RenderServerList();

    CHECK(g_MuServerListUI.GetUndoLabel() == "Add VIP type New VIP type");
    g_MuServerListUI.Undo();
    CHECK_FALSE(g_MuServerListUI.CanUndo());

    Press(TabId("###Groups"));
    RenderServerList();
}

// The Server list root of the Colors window writes a theme color into a theme folder's tokens.ini: the token's line
// when the file has one, else a new line after the [Tokens] section's last; the rest of the file and its line ends
// stay as they are.
TEST_CASE("Server list colors: a theme color is written into a theme's tokens.ini [editor]")
{
    namespace ListColors = MuEditor::ServerList::Colors;
    const std::filesystem::path folder = std::filesystem::temp_directory_path() /
                                         ("mu_test_theme_tokens_" + std::to_string(std::random_device{}()));
    const std::filesystem::path previous = std::filesystem::current_path();
    std::filesystem::create_directories(folder / "Data/Interface/RmlUi/themes/plain");
    const std::filesystem::path tokens = folder / "Data/Interface/RmlUi/themes/plain/tokens.ini";
    {
        std::ofstream file(tokens, std::ios::binary);
        file << "; a theme\r\n[Tokens]\r\nserver-button=#ffff79\r\n; the load\r\nserver-load-empty=rgba(0, 0, 0, 160)\r\n"
                "\r\n[Other]\r\nkey=1\r\n";
    }
    std::filesystem::current_path(folder);

    CHECK(ListColors::WriteThemeToken("plain", "server-button", "#123456"));
    CHECK(ListColors::WriteThemeToken("plain", "server-button-silver", "#abcdef"));
    CHECK_FALSE(ListColors::WriteThemeToken("missing", "server-button", "#123456"));

    std::filesystem::current_path(previous);
    std::ifstream file(tokens, std::ios::binary);
    std::stringstream text;
    text << file.rdbuf();
    CHECK(text.str() == "; a theme\r\n[Tokens]\r\nserver-button=#123456\r\n; the load\r\nserver-load-empty=rgba(0, 0, 0, 160)\r\n"
                        "server-button-silver=#abcdef\r\n\r\n[Other]\r\nkey=1\r\n");
    file.close();
    std::error_code ignored;
    std::filesystem::remove_all(folder, ignored);
}

TEST_CASE("Server list colors: a token is set in tokens.ini's text [editor]")
{
    namespace ListColors = MuEditor::ServerList::Colors;
    // The line that has it, else after the section's last value line, else a new section at the end.
    CHECK(ListColors::SetTokenInText("[Tokens]\na=1\nb=2\n", "a", "9") == "[Tokens]\na=9\nb=2\n");
    CHECK(ListColors::SetTokenInText("[Tokens]\na=1\n; end\n\n[Other]\n", "c", "3") ==
          "[Tokens]\na=1\nc=3\n; end\n\n[Other]\n");
    CHECK(ListColors::SetTokenInText("[Other]\nkey=1", "c", "3") == "[Other]\nkey=1\n[Tokens]\nc=3");
    CHECK(ListColors::SetTokenInText("", "c", "3") == "[Tokens]\nc=3");
    // A key of another section isn't the token.
    CHECK(ListColors::SetTokenInText("[Other]\na=1\n[Tokens]\n", "a", "9") == "[Other]\na=1\n[Tokens]\na=9\n");
    CHECK(ListColors::SetTokenInText("[Tokens]\r\na=1\r\n", "a", "9") == "[Tokens]\r\na=9\r\n");
}

TEST_CASE("Server list colors: a token color keeps its alpha [editor]")
{
    namespace ListColors = MuEditor::ServerList::Colors;
    const std::optional<ListColors::TokenColor> rgba = ListColors::ParseTokenColor("rgba(0, 0, 0, 160)");
    REQUIRE(rgba.has_value());
    CHECK(rgba->alpha == 160);
    CHECK(ListColors::FormatTokenColor(*rgba) == "rgba(0, 0, 0, 160)");
    const std::optional<ListColors::TokenColor> hex = ListColors::ParseTokenColor("#FFB400");
    REQUIRE(hex.has_value());
    CHECK(ListColors::FormatTokenColor(*hex) == "#ffb400");
    CHECK_FALSE(ListColors::ParseTokenColor("orange").has_value());
}

#endif // _EDITOR
