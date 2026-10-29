#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

// The editor's map picker (monsters design, D67).

#include "TestImGuiContext.h"

#include "Core/Text/Utf8.h"
#include "UI/Common/MapPicker.h"
#include "World/MapInfra/MapManager.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <set>
#include <string>
#include <vector>

namespace MapPicker = MuEditor::MapPicker;

namespace
{
std::string GameName(int map)
{
    return Core::Text::ToUtf8(gMapManager.GetMapName(map));
}

const MapPicker::Row& RowOf(int nameMap)
{
    for (const MapPicker::Row& row : MapPicker::GetRows())
    {
        if (row.nameMap == nameMap)
        {
            return row;
        }
    }
    FAIL("no row named by map " << nameMap);
    return MapPicker::GetRows().front();
}

size_t IndexOf(int nameMap)
{
    const std::vector<MapPicker::Row>& rows = MapPicker::GetRows();
    for (size_t index = 0; index < rows.size(); ++index)
    {
        if (rows[index].nameMap == nameMap)
        {
            return index;
        }
    }
    FAIL("no row named by map " << nameMap);
    return 0;
}
} // namespace

TEST_CASE("Map picker: each map a server sends once, the rounds of an event under one name [editor][maps]")
{
    std::set<int> seen;
    for (const MapPicker::Row& row : MapPicker::GetRows())
    {
        for (const MapPicker::Member& member : row.members)
        {
            INFO("map " << member.map);
            CHECK(seen.insert(member.map).second);
            CHECK(MapPicker::NameOf(member.map).rfind("map ", 0) != 0);
            CHECK(member.round.empty() == (row.members.size() == 1));
        }
    }
    // 27 maps of their own and the 40 rounds of 8 events.
    CHECK(seen.size() == 67);

    std::vector<int> devilSquare;
    for (const MapPicker::Member& member : RowOf(WD_9DEVILSQUARE).members)
    {
        devilSquare.push_back(member.map);
    }
    CHECK(devilSquare == std::vector<int>{WD_9DEVILSQUARE, 32});
    CHECK(RowOf(WD_24HELLAS).members.back().map == WD_24HELLAS_7);
    CHECK(RowOf(WD_11BLOODCASTLE1).members.back().map == WD_52BLOODCASTLE_MASTER_LEVEL);
    CHECK(RowOf(WD_18CHAOS_CASTLE).members.back().map == WD_53CAOSCASTLE_MASTER_LEVEL);
    // The login and character scenes are no maps a server sends.
    CHECK_FALSE(seen.contains(WD_55LOGINSCENE));
    CHECK_FALSE(seen.contains(WD_73NEW_LOGIN_SCENE));
}

TEST_CASE("Map picker: a list says its maps, a whole event by its name (D67) [editor][maps]")
{
    CHECK(MapPicker::Describe({}, "every map") == "every map");
    CHECK(MapPicker::Describe({32}, "") == GameName(WD_9DEVILSQUARE) + " 5-7");
    CHECK(MapPicker::Describe({32, WD_0LORENCIA}, "") ==
          GameName(WD_0LORENCIA) + ", " + GameName(WD_9DEVILSQUARE) + " 5-7");
    CHECK(MapPicker::Describe({WD_9DEVILSQUARE, 32}, "") == GameName(WD_9DEVILSQUARE));
    CHECK(MapPicker::Describe({24, 25, 26, 27, 28, 29, 36}, "") == GameName(WD_24HELLAS));
    CHECK(MapPicker::Describe({36}, "") == GameName(WD_24HELLAS) + " 7");
    CHECK(MapPicker::Describe({999}, "") == "map 999");
}

TEST_CASE("Map picker: a row's server numbers and world folders (D67) [editor][maps]")
{
    CHECK(MapPicker::InfoOf(RowOf(WD_9DEVILSQUARE)) == "server 9, 32; World10");
    CHECK(MapPicker::InfoOf(RowOf(WD_11BLOODCASTLE1)) == "server 11-17, 52; World12");
    CHECK(MapPicker::InfoOf(RowOf(WD_24HELLAS)) == "server 24-29, 36; World25");
    CHECK(MapPicker::InfoOf(RowOf(WD_65DOPPLEGANGER1)) == "server 65-68; World66, World67, World68, World69");
    CHECK(MapPicker::InfoOf(RowOf(WD_0LORENCIA)) == "server 0; World1");
}

TEST_CASE("Map picker: maps are added and removed, the list sorted without repeats [editor][maps]")
{
    std::vector<int> maps = {WD_56MAP_SWAMP_OF_QUIET};
    MapPicker::SetMaps(maps, RowOf(WD_9DEVILSQUARE).members, true);
    CHECK(maps == std::vector<int>{WD_9DEVILSQUARE, 32, WD_56MAP_SWAMP_OF_QUIET});
    MapPicker::SetMaps(maps, RowOf(WD_9DEVILSQUARE).members, true);
    CHECK(maps == std::vector<int>{WD_9DEVILSQUARE, 32, WD_56MAP_SWAMP_OF_QUIET});
    MapPicker::SetMaps(maps, {{WD_9DEVILSQUARE, "1-4"}}, false);
    CHECK(maps == std::vector<int>{32, WD_56MAP_SWAMP_OF_QUIET});
}

TEST_CASE("Map picker: the button opens it, an event's box picks all its rounds [editor][maps]")
{
    TestImGuiContext context;
    std::vector<int> maps;
    bool changed = false;
    const auto render = [&]
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(600.0f, 500.0f));
        ImGui::Begin("Maps test");
        changed = MapPicker::Render("Picker test", maps, "every map") || changed;
        ImGui::End();
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    render();
    const ImGuiWindow* window = ImGui::FindWindowByName("Maps test");
    REQUIRE(window != nullptr);
    const ImGuiID scope = ImGui::GetIDWithSeed("Picker test", nullptr, window->ID);
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed("###Maps", nullptr, scope));
    render();
    render();
    const ImGuiID popupId = ImGui::GetIDWithSeed("Picker", nullptr, scope);
    char name[32];
    ImFormatString(name, sizeof(name), "##Popup_%08x", popupId);
    const ImGuiWindow* popup = ImGui::FindWindowByName(name);
    REQUIRE(popup != nullptr);
    CHECK_FALSE(changed);

    const ImGuiID row = ImGui::GetIDWithSeed(static_cast<int>(IndexOf(WD_9DEVILSQUARE)), popup->ID);
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed(GameName(WD_9DEVILSQUARE).c_str(), nullptr, row));
    render();
    render();
    CHECK(changed);
    CHECK(maps == std::vector<int>{WD_9DEVILSQUARE, 32});
}

#endif // _EDITOR
