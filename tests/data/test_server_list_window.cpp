#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "TestImGuiContext.h"

#include "Data/GameData/ServerListData/ServerListJson.h"
#include "I18N/All.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/ServerList/MuServerListUI.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <cstring>
#include <filesystem>
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
        std::vector<ServerGroup> groups;
        int id = 0;
        for (const char* name : {"Valhalla", "Helheim", "Midgard"})
        {
            ServerGroup group;
            group.id = id++;
            group.name = name;
            groups.push_back(group);
        }
        std::vector<Data::Items::ItemDataIssue> issues;
        REQUIRE(Data::ServerList::SaveServerListFile(m_directory / Data::ServerList::ServerListFile, groups, issues));
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

// One of the window's panes: "##Groups" (the list and its buttons) or
// "##Group" (the selected group). ImGui names a child window
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

#endif // _EDITOR
