#include "stdafx.h"

#ifdef _EDITOR

#include "MapPicker.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Core/Text/Utf8.h"
#include "World/MapInfra/MapManager.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <utility>

namespace MuEditor::MapPicker
{
namespace
{
// Pixel height before the editor UI scale; the picker scrolls beyond it.
constexpr float MaxPickerHeight = 420.0f;

// The maps a server sends, by the server's numbers; the rounds of an event
// as one row. The scenes of the login and character screens are left out.
std::vector<Row> MakeRows()
{
    std::vector<Row> rows;
    const auto single = [&](int map) { rows.push_back({map, {{map, ""}}}); };
    const auto rounds = [&](std::vector<Member> members)
    {
        const int first = members.front().map;
        rows.push_back({first, std::move(members)});
    };
    for (const int map :
         {WD_0LORENCIA, WD_1DUNGEON, WD_2DEVIAS, WD_3NORIA, WD_4LOSTTOWER, WD_6STADIUM, WD_7ATLANSE, WD_8TARKAN})
    {
        single(map);
    }
    rounds({{WD_9DEVILSQUARE, "1-4"}, {32, "5-7"}});
    single(WD_10HEAVEN);
    rounds({{WD_11BLOODCASTLE1, "1"},
            {WD_11BLOODCASTLE1 + 1, "2"},
            {WD_11BLOODCASTLE1 + 2, "3"},
            {WD_11BLOODCASTLE1 + 3, "4"},
            {WD_11BLOODCASTLE1 + 4, "5"},
            {WD_11BLOODCASTLE1 + 5, "6"},
            {WD_11BLOODCASTLE1 + 6, "7"},
            {WD_52BLOODCASTLE_MASTER_LEVEL, "8"}});
    rounds({{WD_18CHAOS_CASTLE, "1"},
            {WD_18CHAOS_CASTLE + 1, "2"},
            {WD_18CHAOS_CASTLE + 2, "3"},
            {WD_18CHAOS_CASTLE + 3, "4"},
            {WD_18CHAOS_CASTLE + 4, "5"},
            {WD_18CHAOS_CASTLE + 5, "6"},
            {WD_53CAOSCASTLE_MASTER_LEVEL, "7"}});
    rounds({{WD_24HELLAS, "1"},
            {WD_24HELLAS + 1, "2"},
            {WD_24HELLAS + 2, "3"},
            {WD_24HELLAS + 3, "4"},
            {WD_24HELLAS + 4, "5"},
            {WD_24HELLAS + 5, "6"},
            {WD_24HELLAS_7, "7"}});
    for (const int map :
         {WD_30BATTLECASTLE, WD_31HUNTING_GROUND, WD_33AIDA, WD_34CRYWOLF_1ST, WD_37KANTURU_1ST, WD_38KANTURU_2ND,
          WD_39KANTURU_3RD, WD_40AREA_FOR_GM, WD_41CHANGEUP3RD_1ST, WD_42CHANGEUP3RD_2ND})
    {
        single(map);
    }
    rounds({{WD_45CURSEDTEMPLE_LV1, "1"},
            {WD_45CURSEDTEMPLE_LV2, "2"},
            {WD_45CURSEDTEMPLE_LV3, "3"},
            {WD_45CURSEDTEMPLE_LV4, "4"},
            {WD_45CURSEDTEMPLE_LV5, "5"},
            {WD_45CURSEDTEMPLE_LV6, "6"}});
    for (const int map : {WD_51HOME_6TH_CHAR, WD_56MAP_SWAMP_OF_QUIET, WD_57ICECITY, WD_58ICECITY_BOSS, WD_62SANTA_TOWN,
                          WD_63PK_FIELD, WD_64DUELARENA})
    {
        single(map);
    }
    rounds(
        {{WD_65DOPPLEGANGER1, "1"}, {WD_66DOPPLEGANGER2, "2"}, {WD_67DOPPLEGANGER3, "3"}, {WD_68DOPPLEGANGER4, "4"}});
    rounds({{WD_69EMPIREGUARDIAN1, "1"},
            {WD_70EMPIREGUARDIAN2, "2"},
            {WD_71EMPIREGUARDIAN3, "3"},
            {WD_72EMPIREGUARDIAN4, "4"}});
    single(WD_79UNITEDMARKETPLACE);
    rounds({{WD_80KARUTAN1, "1"}, {WD_81KARUTAN2, "2"}});
    return rows;
}

const Row* FindRow(int map)
{
    for (const Row& row : GetRows())
    {
        for (const Member& member : row.members)
        {
            if (member.map == map)
            {
                return &row;
            }
        }
    }
    return nullptr;
}

std::string RowName(const Row& row)
{
    return Core::Text::ToUtf8(gMapManager.GetMapName(row.nameMap));
}

bool Contains(const std::vector<int>& maps, int map)
{
    return std::find(maps.begin(), maps.end(), map) != maps.end();
}

size_t CountChosen(const Row& row, const std::vector<int>& maps)
{
    return static_cast<size_t>(std::count_if(row.members.begin(), row.members.end(),
                                             [&](const Member& member) { return Contains(maps, member.map); }));
}

// "11-17, 52": runs of three or more numbers as a range.
std::string NumberList(std::vector<int> numbers)
{
    std::sort(numbers.begin(), numbers.end());
    numbers.erase(std::unique(numbers.begin(), numbers.end()), numbers.end());
    std::string text;
    for (size_t i = 0; i < numbers.size();)
    {
        size_t end = i;
        while (end + 1 < numbers.size() && numbers[end + 1] == numbers[end] + 1)
        {
            ++end;
        }
        text += text.empty() ? "" : ", ";
        if (end - i >= 2)
        {
            text += std::to_string(numbers[i]) + "-" + std::to_string(numbers[end]);
            i = end + 1;
        }
        else
        {
            text += std::to_string(numbers[i]);
            ++i;
        }
    }
    return text;
}

bool RenderRow(const Row& row, std::vector<int>& maps)
{
    bool changed = false;
    const std::string name = RowName(row);
    const size_t chosen = CountChosen(row, maps);
    if (row.members.size() == 1)
    {
        bool on = chosen == 1;
        if (ImGui::Checkbox(name.c_str(), &on))
        {
            SetMaps(maps, row.members, on);
            changed = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", InfoOf(row).c_str());
        return changed;
    }
    if (chosen > 0)
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    }
    const bool open = ImGui::TreeNodeEx("##Rounds", ImGuiTreeNodeFlags_NoTreePushOnOpen);
    ImGui::SameLine();
    bool all = chosen == row.members.size();
    ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, chosen > 0 && !all);
    if (ImGui::Checkbox(name.c_str(), &all))
    {
        SetMaps(maps, row.members, all);
        changed = true;
    }
    ImGui::PopItemFlag();
    ImGui::SameLine();
    ImGui::TextDisabled("%s", InfoOf(row).c_str());
    if (!open)
    {
        return changed;
    }
    ImGui::Indent();
    for (const Member& member : row.members)
    {
        ImGui::PushID(member.map);
        bool on = Contains(maps, member.map);
        if (ImGui::Checkbox(member.round.c_str(), &on))
        {
            SetMaps(maps, {member}, on);
            changed = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", InfoOf({member.map, {member}}).c_str());
        ImGui::PopID();
    }
    ImGui::Unindent();
    return changed;
}
} // namespace

const std::vector<Row>& GetRows()
{
    static const std::vector<Row> rows = MakeRows();
    return rows;
}

std::string NameOf(int map)
{
    const Row* row = FindRow(map);
    if (row == nullptr)
    {
        return "map " + std::to_string(map);
    }
    if (row->members.size() == 1)
    {
        return RowName(*row);
    }
    const auto member = std::find_if(row->members.begin(), row->members.end(),
                                     [map](const Member& candidate) { return candidate.map == map; });
    return RowName(*row) + " " + member->round;
}

std::string Describe(const std::vector<int>& maps, const char* whenEmpty)
{
    if (maps.empty())
    {
        return whenEmpty;
    }
    std::vector<int> sorted = maps;
    std::sort(sorted.begin(), sorted.end());
    std::string text;
    std::vector<const Row*> wholeRows;
    for (const int map : sorted)
    {
        const Row* row = FindRow(map);
        std::string part;
        if (row != nullptr && row->members.size() > 1 && CountChosen(*row, sorted) == row->members.size())
        {
            // All rounds chosen: the map's name once.
            if (std::find(wholeRows.begin(), wholeRows.end(), row) != wholeRows.end())
            {
                continue;
            }
            wholeRows.push_back(row);
            part = RowName(*row);
        }
        else
        {
            part = NameOf(map);
        }
        text += (text.empty() ? "" : ", ") + part;
    }
    return text;
}

std::string InfoOf(const Row& row)
{
    std::vector<int> servers;
    std::vector<int> folders;
    for (const Member& member : row.members)
    {
        servers.push_back(member.map);
        folders.push_back(gMapManager.GetFilesFolder(member.map));
    }
    std::sort(folders.begin(), folders.end());
    folders.erase(std::unique(folders.begin(), folders.end()), folders.end());
    std::string world;
    for (const int folder : folders)
    {
        world += (world.empty() ? "World" : ", World") + std::to_string(folder);
    }
    return "server " + NumberList(servers) + "; " + world;
}

void SetMaps(std::vector<int>& maps, const std::vector<Member>& members, bool on)
{
    for (const Member& member : members)
    {
        const bool chosen = Contains(maps, member.map);
        if (on && !chosen)
        {
            maps.push_back(member.map);
        }
        else if (!on && chosen)
        {
            maps.erase(std::remove(maps.begin(), maps.end(), member.map), maps.end());
        }
    }
    std::sort(maps.begin(), maps.end());
}

bool Render(const char* id, std::vector<int>& maps, const char* whenEmpty)
{
    bool changed = false;
    ImGui::PushID(id);
    const std::string label = Describe(maps, whenEmpty) + "###Maps";
    if (ImGui::Button(label.c_str()))
    {
        ImGui::OpenPopup("Picker");
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f),
                                        ImVec2(FLT_MAX, MaxPickerHeight * g_MuEditorCore.GetUIScale()));
    if (ImGui::BeginPopup("Picker"))
    {
        const std::vector<Row>& rows = GetRows();
        for (size_t index = 0; index < rows.size(); ++index)
        {
            ImGui::PushID(static_cast<int>(index));
            changed = RenderRow(rows[index], maps) || changed;
            ImGui::PopID();
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
    return changed;
}
} // namespace MuEditor::MapPicker

#endif // _EDITOR
