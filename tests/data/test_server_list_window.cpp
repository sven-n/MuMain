#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "TestImGuiContext.h"

#include "Config/MuEditorConfig.h"
#include "Data/GameData/ServerListData/ServerListJson.h"
#include "I18N/All.h"
#include "Network/Server/ServerListManager.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/ServerList/MuServerListUI.h"
#include "UI/ServerList/ServerListColors.h"
#include "UI/ServerList/ServerListOfflinePreviewUI.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
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

// The id of a widget in a row of the selected group's channel table (RenderChannelRow pushes the channel from 0).
ImGuiID ChannelRowId(int channel, const char* label)
{
    const ImGuiID table = PaneId("##Group", "##Channels");
    return ImGui::GetIDWithSeed(label, nullptr, ImGui::GetIDWithSeed(channel, table));
}

// The editor's preview of the offline servers on for one test (as MuEditor.ini has it, so nothing writes the
// file); after it every channel is shown again without a load of its own, and the preview is as before.
class OfflinePreviewOn
{
public:
    static constexpr int Load = 50;

    OfflinePreviewOn()
        : m_show(g_ServerListManager->IsShowingOfflineServers()),
          m_percent(g_ServerListManager->GetOfflineServerPercent()),
          m_configShow(g_MuEditorConfig.GetShowOfflineServers()), m_configLoad(g_MuEditorConfig.GetOfflineServerLoad())
    {
        g_MuEditorConfig.SetOfflineServers(true, Load);
        g_ServerListManager->SetOfflinePreview(true, Load);
    }

    ~OfflinePreviewOn()
    {
        for (int serverId = 0; serverId < GroupCount * Data::ServerList::MaxChannelsPerGroup; ++serverId)
        {
            g_ServerListManager->SetOfflineServerShown(serverId, true);
            g_ServerListManager->SetOfflineServerLoad(serverId, std::nullopt);
        }
        g_ServerListManager->SetOfflinePreview(m_show, m_percent);
        g_MuEditorConfig.SetOfflineServers(m_configShow, m_configLoad);
    }

    OfflinePreviewOn(const OfflinePreviewOn&) = delete;
    OfflinePreviewOn& operator=(const OfflinePreviewOn&) = delete;

private:
    // Valhalla, Helheim and Midgard (ServerListFolder).
    static constexpr int GroupCount = 3;

    bool m_show;
    int m_percent;
    bool m_configShow;
    int m_configLoad;
};

constexpr const char* OfflineChannelWindow = "Offline channel";

// The preview's controls of the channel `serverId` alone in a window, as the channel table draws them; the
// rectangle of the last one (the load slider while the channel follows Their load).
ImRect RenderOfflineChannel(int serverId)
{
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin(OfflineChannelWindow);
    MuEditor::ServerList::OfflinePreview::RenderChannel(serverId);
    const ImRect last(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    ImGui::End();
    ImGui::Render();
    return last;
}

ImGuiID OfflineChannelId(const char* label)
{
    return ImGui::GetIDWithSeed(label, nullptr, ImGui::FindWindowByName(OfflineChannelWindow)->ID);
}

void SetMouseButton(bool down, int serverId)
{
    ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Left, down);
    RenderOfflineChannel(serverId);
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

TEST_CASE("Server List window: an offline channel's Show box leaves it out, All and None the group's [editor]")
{
    I18N::SetLocale("en");
    ServerListFolder folder;
    TestImGuiContext context;
    OfflinePreviewOn preview;
    StartWithValhalla();

    // Valhalla's channel 4; the test has no connect server, so every channel is offline.
    constexpr int ServerId = 3;
    REQUIRE(g_ServerListManager->IsOfflineServerShown(ServerId));
    Press(ChannelRowId(ServerId, I18N::Editor::ShowOfflineChannel));
    RenderServerList();
    CHECK_FALSE(g_ServerListManager->IsOfflineServerShown(ServerId));
    CHECK(g_ServerListManager->IsOfflineServerShown(ServerId + 1));
    // The preview is not part of the list.
    CHECK_FALSE(g_MuServerListUI.CanUndo());

    Press(PaneId("##Group", I18N::Editor::NoOfflineChannel));
    for (int channel = 0; channel < Data::ServerList::MaxChannelsPerGroup; ++channel)
    {
        CHECK_FALSE(g_ServerListManager->IsOfflineServerShown(channel));
    }
    // Helheim's stay.
    CHECK(g_ServerListManager->IsOfflineServerShown(Data::ServerList::MaxChannelsPerGroup));
    Press(PaneId("##Group", I18N::Editor::AllOfflineChannels));
    CHECK(g_ServerListManager->IsOfflineServerShown(ServerId));
    CHECK_FALSE(g_MuServerListUI.CanUndo());
}

TEST_CASE("Server List window: a double-click on an offline channel's load types its own one [editor]")
{
    I18N::SetLocale("en");
    TestImGuiContext context;
    OfflinePreviewOn preview;
    constexpr int ServerId = 3;
    const ImRect slider = RenderOfflineChannel(ServerId);
    const ImGuiID sliderId = OfflineChannelId("##OfflineLoad");
    REQUIRE(slider.GetWidth() > 0.0f);

    // At a quarter of the slider: a click alone would set about 35 %.
    ImGui::GetIO().AddMousePosEvent(slider.Min.x + slider.GetWidth() / 4.0f, slider.GetCenter().y);
    RenderOfflineChannel(ServerId);
    SetMouseButton(true, ServerId);
    SetMouseButton(false, ServerId);
    SetMouseButton(true, ServerId);
    // The text field opens in the frame after the double-click, with the load from before the first click.
    RenderOfflineChannel(ServerId);
    CHECK(ImGui::TempInputIsActive(sliderId));
    CHECK(g_ServerListManager->GetOfflineServerLoad(ServerId) == OfflinePreviewOn::Load);
    SetMouseButton(false, ServerId);

    ImGui::GetIO().AddInputCharactersUTF8("120");
    RenderOfflineChannel(ServerId);
    ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, true);
    RenderOfflineChannel(ServerId);
    ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, false);
    RenderOfflineChannel(ServerId);
    CHECK_FALSE(ImGui::TempInputIsActive(sliderId));
    CHECK(g_ServerListManager->GetOfflineServerLoad(ServerId) == 120);
}

TEST_CASE("Server List window: the x next to an offline channel's own load goes back to Their load [editor]")
{
    I18N::SetLocale("en");
    TestImGuiContext context;
    OfflinePreviewOn preview;
    constexpr int ServerId = 3;
    g_ServerListManager->SetOfflineServerLoad(ServerId, 120);
    RenderOfflineChannel(ServerId);

    ImGui::ActivateItemByID(OfflineChannelId("\xC3\x97##DropOwnLoad"));
    RenderOfflineChannel(ServerId);
    CHECK_FALSE(g_ServerListManager->GetOfflineServerLoad(ServerId).has_value());
}

// The Colors window counts a color's users each frame without their labels; the count is the list's length.
TEST_CASE("Server list colors: a color's count is the number of its users [editor]")
{
    I18N::SetLocale("en");
    ServerListFolder folder;
    TestImGuiContext context;
    StartWithValhalla();

    for (const char* color : {"default", "gray", "orange", "silver", "server-load-base", "none"})
    {
        CAPTURE(color);
        CHECK(g_MuServerListUI.CountColorUsers(color) ==
              static_cast<int>(g_MuServerListUI.DescribeColorUsers(color).size()));
        CHECK(g_MuServerListUI.CountThemeBarPartUsers(color) ==
              static_cast<int>(g_MuServerListUI.DescribeThemeBarPartUsers(color).size()));
    }
    // The built-in load: both load texts in the button's own color, which isn't the server-button token "default"
    // is, and the bar in the theme's tokens.
    CHECK(g_MuServerListUI.CountColorUsers("default") == 0);
    for (const char* token : {"server-load-base", "server-load-full", "server-load-empty"})
    {
        CHECK(g_MuServerListUI.CountThemeBarPartUsers(token) == 1);
    }
    // A rule in "default" is drawn in it; a load text in a theme color is that color's.
    Data::ServerList::ServerListData& list = g_MuServerListUI.GetEdits().EditList();
    list.buttonColors.push_back(Data::ServerList::ButtonColorRule{});
    list.load.full.color = "gray";
    CHECK(g_MuServerListUI.CountColorUsers("default") == 1);
    CHECK(g_MuServerListUI.CountColorUsers("gray") == 1);
    CHECK(g_MuServerListUI.DescribeColorUsers("gray").size() == 1);
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
    // Written through a temporary file (Data::WriteTextFile), which takes the file's place.
    CHECK_FALSE(std::filesystem::exists(tokens.string() + ".tmp"));
    // A theme that inherits all its tokens gets a tokens.ini of its own.
    std::filesystem::create_directories("Data/Interface/RmlUi/themes/inherits");
    CHECK(ListColors::WriteThemeToken("inherits", "server-button", "#123456"));

    std::filesystem::current_path(previous);
    const auto readFile = [](const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        std::stringstream text;
        text << file.rdbuf();
        return text.str();
    };
    CHECK(readFile(tokens) ==
          "; a theme\r\n[Tokens]\r\nserver-button=#123456\r\n; the load\r\nserver-load-empty=rgba(0, 0, 0, 160)\r\n"
          "server-button-silver=#abcdef\r\n\r\n[Other]\r\nkey=1\r\n");
    CHECK(readFile(folder / "Data/Interface/RmlUi/themes/inherits/tokens.ini") == "[Tokens]\nserver-button=#123456\n");
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

TEST_CASE("Server list colors: tokens.ini's lines count as the themes read them [editor]")
{
    namespace ListColors = MuEditor::ServerList::Colors;
    // Spaces around the key, another case, a header with spaces, a byte order mark.
    CHECK(ListColors::SetTokenInText("[ tokens ]\nServer-Load-Base = #00bd8b\n", "server-load-base", "#123456") ==
          "[ tokens ]\nserver-load-base=#123456\n");
    CHECK(ListColors::SetTokenInText("\xef\xbb\xbf[Tokens]\na=1\n", "b", "2") == "\xef\xbb\xbf[Tokens]\na=1\nb=2\n");
    // A '#' comment is no value line to add after; a key the section has twice takes the value in both.
    CHECK(ListColors::SetTokenInText("[Tokens]\na=1\n# later\n", "b", "2") == "[Tokens]\na=1\nb=2\n# later\n");
    CHECK(ListColors::SetTokenInText("[Tokens]\na=1\n[Tokens]\na=2\n", "a", "9") == "[Tokens]\na=9\n[Tokens]\na=9\n");
}

TEST_CASE("Server list colors: a shade goes to the swatch and back unchanged [editor]")
{
    namespace ListColors = MuEditor::ServerList::Colors;
    const std::optional<ImVec4> swatch = ListColors::ParseShade("#ffb400");
    REQUIRE(swatch.has_value());
    const float rgb[3] = {swatch->x, swatch->y, swatch->z};
    CHECK(Core::Color::FormatRgb(ListColors::ToRgb(rgb)) == "#FFB400");
    CHECK_FALSE(ListColors::ParseShade("#ffb40").has_value());
    CHECK_FALSE(ListColors::ParseShade("ffb400").has_value());
    const float outOfRange[3] = {1.5f, -0.5f, 0.5f};
    CHECK(ListColors::ToRgb(outOfRange) == Core::Color::Rgb{255, 0, 128});
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
    // Spaces or none around the numbers.
    REQUIRE(ListColors::ParseTokenColor("rgba(1,2,3,4)").has_value());
    CHECK(ListColors::ParseTokenColor("rgba( 1 , 2 , 3 , 4 )")->alpha == 4);
    // Only four whole numbers from 0 to 255: anything else the editor could not write back as the theme has it.
    CHECK_FALSE(ListColors::ParseTokenColor("rgba(0, 0, 0, 0.5)").has_value());
    CHECK_FALSE(ListColors::ParseTokenColor("rgba(1, 2, 3, 4, 5)").has_value());
    CHECK_FALSE(ListColors::ParseTokenColor("rgba(1, 2, 3)").has_value());
    CHECK_FALSE(ListColors::ParseTokenColor("rgba(1 2 3 4)").has_value());
    CHECK_FALSE(ListColors::ParseTokenColor("rgba(1,, 2, 3, 4)").has_value());
    CHECK_FALSE(ListColors::ParseTokenColor("rgba(0, 0, 0, 256)").has_value());
}

#endif // _EDITOR
