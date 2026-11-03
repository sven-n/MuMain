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
using Data::Maps::MapRef;

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

TEST_CASE("Map picker: each map or stage a server sends once, the levels of an event under one name [editor][maps]")
{
    std::set<MapRef> seen;
    std::set<int> maps;
    for (const MapPicker::Row& row : MapPicker::GetRows())
    {
        for (const MapPicker::Member& member : row.members)
        {
            INFO("map " << member.ref.map << " stage " << member.ref.stage);
            CHECK(seen.insert(member.ref).second);
            maps.insert(member.ref.map);
            CHECK(MapPicker::NameOf(member.ref).rfind("map ", 0) != 0);
            CHECK(member.label.empty() == (row.members.size() == 1));
        }
    }
    // 27 maps of their own, the 38 levels of 7 events that are maps, and the 7
    // stages of Devil Square on 2 maps.
    CHECK(seen.size() == 72);
    CHECK(maps.size() == 67);

    std::vector<MapRef> devilSquare;
    for (const MapPicker::Member& member : RowOf(WD_9DEVILSQUARE).members)
    {
        devilSquare.push_back(member.ref);
    }
    CHECK(devilSquare == std::vector<MapRef>{{WD_9DEVILSQUARE, 1},
                                             {WD_9DEVILSQUARE, 2},
                                             {WD_9DEVILSQUARE, 3},
                                             {WD_9DEVILSQUARE, 4},
                                             {32, 5},
                                             {32, 6},
                                             {32, 7}});
    CHECK(MapPicker::StagesOf(WD_9DEVILSQUARE) == std::vector<int>{1, 2, 3, 4});
    CHECK(MapPicker::StagesOf(32) == std::vector<int>{5, 6, 7});
    CHECK(MapPicker::StagesOf(WD_11BLOODCASTLE1).empty());
    CHECK(RowOf(WD_24HELLAS).members.back().ref == MapRef{WD_24HELLAS_7});
    CHECK(RowOf(WD_11BLOODCASTLE1).members.back().ref == MapRef{WD_52BLOODCASTLE_MASTER_LEVEL});
    CHECK(RowOf(WD_18CHAOS_CASTLE).members.back().ref == MapRef{WD_53CAOSCASTLE_MASTER_LEVEL});
    // The login and character scenes are no maps a server sends.
    CHECK_FALSE(maps.contains(WD_55LOGINSCENE));
    CHECK_FALSE(maps.contains(WD_73NEW_LOGIN_SCENE));
}

TEST_CASE("Map picker: the name of a map or stage [editor][maps]")
{
    const std::string devilSquare = GameName(WD_9DEVILSQUARE);
    CHECK(MapPicker::NameOf({WD_9DEVILSQUARE, 2}) == devilSquare + " 2");
    CHECK(MapPicker::NameOf({32, 6}) == devilSquare + " 6");
    // A map without a stage names all of its stages.
    CHECK(MapPicker::NameOf({WD_9DEVILSQUARE}) == devilSquare + " 1-4");
    CHECK(MapPicker::NameOf({32}) == devilSquare + " 5-7");
    CHECK(MapPicker::NameOf({WD_11BLOODCASTLE1 + 2}) == GameName(WD_11BLOODCASTLE1) + " 3");
    CHECK(MapPicker::NameOf({WD_0LORENCIA}) == GameName(WD_0LORENCIA));
    // Stages the picker does not know, from hand-written data.
    CHECK(MapPicker::NameOf({WD_0LORENCIA, 3}) == GameName(WD_0LORENCIA) + " stage 3");
    CHECK(MapPicker::NameOf({WD_9DEVILSQUARE, 5}) == devilSquare + " 1-4 stage 5");
    CHECK(MapPicker::NameOf({999, 2}) == "map 999 stage 2");
}

TEST_CASE("Map picker: a list says its maps, a whole event by its name (D67) [editor][maps]")
{
    CHECK(MapPicker::Describe({}, "every map") == "every map");
    CHECK(MapPicker::Describe({{32}}, "") == GameName(WD_9DEVILSQUARE) + " 5-7");
    CHECK(MapPicker::Describe({{32}, {WD_0LORENCIA}}, "") ==
          GameName(WD_0LORENCIA) + ", " + GameName(WD_9DEVILSQUARE) + " 5-7");
    CHECK(MapPicker::Describe({{WD_9DEVILSQUARE}, {32}}, "") == GameName(WD_9DEVILSQUARE));
    CHECK(MapPicker::Describe({{WD_9DEVILSQUARE, 2}, {32, 7}}, "") ==
          GameName(WD_9DEVILSQUARE) + " 2, " + GameName(WD_9DEVILSQUARE) + " 7");
    // Every stage one by one is the whole event too.
    CHECK(MapPicker::Describe(
              {{WD_9DEVILSQUARE, 1}, {WD_9DEVILSQUARE, 2}, {WD_9DEVILSQUARE, 3}, {WD_9DEVILSQUARE, 4}, {32}}, "") ==
          GameName(WD_9DEVILSQUARE));
    CHECK(MapPicker::Describe({{24}, {25}, {26}, {27}, {28}, {29}, {36}}, "") == GameName(WD_24HELLAS));
    CHECK(MapPicker::Describe({{36}}, "") == GameName(WD_24HELLAS) + " 7");
    CHECK(MapPicker::Describe({{999}}, "") == "map 999");
}

TEST_CASE("Map picker: a row's server numbers and world folders (D67) [editor][maps]")
{
    CHECK(MapPicker::InfoOf(RowOf(WD_9DEVILSQUARE)) == "server 9 stages 1-4, 32 stages 5-7; World10");
    CHECK(MapPicker::InfoOf(RowOf(WD_11BLOODCASTLE1)) == "server 11-17, 52; World12");
    CHECK(MapPicker::InfoOf(RowOf(WD_24HELLAS)) == "server 24-29, 36; World25");
    CHECK(MapPicker::InfoOf(RowOf(WD_65DOPPLEGANGER1)) == "server 65-68; World66, World67, World68, World69");
    CHECK(MapPicker::InfoOf(RowOf(WD_0LORENCIA)) == "server 0; World1");
}

TEST_CASE("Map picker: maps are added and removed, the list sorted without repeats [editor][maps]")
{
    const std::vector<MapPicker::Member> lorencia = {{MapRef{WD_0LORENCIA}, ""}};
    std::vector<MapRef> maps = {{WD_56MAP_SWAMP_OF_QUIET}};
    MapPicker::SetMaps(maps, RowOf(WD_11BLOODCASTLE1).members, true);
    CHECK(maps.size() == 9);
    CHECK(maps.front() == MapRef{WD_11BLOODCASTLE1});
    MapPicker::SetMaps(maps, RowOf(WD_11BLOODCASTLE1).members, false);
    CHECK(maps == std::vector<MapRef>{{WD_56MAP_SWAMP_OF_QUIET}});
    MapPicker::SetMaps(maps, lorencia, true);
    MapPicker::SetMaps(maps, lorencia, true);
    CHECK(maps == std::vector<MapRef>{{WD_0LORENCIA}, {WD_56MAP_SWAMP_OF_QUIET}});
    // A map takes the stages hand-written data gives it with it.
    maps.push_back({WD_0LORENCIA, 3});
    MapPicker::SetMaps(maps, lorencia, false);
    CHECK(maps == std::vector<MapRef>{{WD_56MAP_SWAMP_OF_QUIET}});
}

TEST_CASE("Map picker: any stage of Devil Square, a map with every stage chosen without a stage [editor][maps]")
{
    const std::vector<MapPicker::Member>& stages = RowOf(WD_9DEVILSQUARE).members;
    const auto stage = [&](int number) { return std::vector<MapPicker::Member>{stages[number - 1]}; };
    std::vector<MapRef> maps = {{WD_56MAP_SWAMP_OF_QUIET}};

    MapPicker::SetMaps(maps, stage(2), true);
    CHECK(maps == std::vector<MapRef>{{WD_9DEVILSQUARE, 2}, {WD_56MAP_SWAMP_OF_QUIET}});
    CHECK(MapPicker::IsChosen(maps, stages[1]));
    CHECK_FALSE(MapPicker::IsChosen(maps, stages[0]));
    MapPicker::SetMaps(maps, stage(6), true);
    CHECK(maps == std::vector<MapRef>{{WD_9DEVILSQUARE, 2}, {32, 6}, {WD_56MAP_SWAMP_OF_QUIET}});

    // The last stage of a map makes it the map.
    MapPicker::SetMaps(maps, stage(5), true);
    MapPicker::SetMaps(maps, stage(7), true);
    CHECK(maps == std::vector<MapRef>{{WD_9DEVILSQUARE, 2}, {32}, {WD_56MAP_SWAMP_OF_QUIET}});
    CHECK(MapPicker::IsChosen(maps, stages[5]));

    // Taking a stage off a map leaves its other stages.
    MapPicker::SetMaps(maps, stage(6), false);
    CHECK(maps == std::vector<MapRef>{{WD_9DEVILSQUARE, 2}, {32, 5}, {32, 7}, {WD_56MAP_SWAMP_OF_QUIET}});
    MapPicker::SetMaps(maps, stage(2), false);
    MapPicker::SetMaps(maps, stage(2), false);
    CHECK(maps == std::vector<MapRef>{{32, 5}, {32, 7}, {WD_56MAP_SWAMP_OF_QUIET}});

    // The event's box: all of it, then nothing of it.
    MapPicker::SetMaps(maps, stages, true);
    CHECK(maps == std::vector<MapRef>{{WD_9DEVILSQUARE}, {32}, {WD_56MAP_SWAMP_OF_QUIET}});
    MapPicker::SetMaps(maps, stages, false);
    CHECK(maps == std::vector<MapRef>{{WD_56MAP_SWAMP_OF_QUIET}});
}

TEST_CASE("Map picker: the button opens it, an event's box picks all its levels, a stage's box one [editor][maps]")
{
    TestImGuiContext context;
    std::vector<MapRef> maps;
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
    CHECK(maps == std::vector<MapRef>{{WD_9DEVILSQUARE}, {32}});

    // A stage's own box: an event with a level chosen shows its levels.
    changed = false;
    const ImGuiID stage = ImGui::GetIDWithSeed(WD_9DEVILSQUARE * (Data::Maps::MaxStage + 1) + 3, row);
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed("3", nullptr, stage));
    render();
    render();
    CHECK(changed);
    CHECK(maps == std::vector<MapRef>{{WD_9DEVILSQUARE, 1}, {WD_9DEVILSQUARE, 2}, {WD_9DEVILSQUARE, 4}, {32}});
}

#endif // _EDITOR
