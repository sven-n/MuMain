#include "stdafx.h"

#ifdef _EDITOR

#include "MapPicker.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Core/Text/Utf8.h"
#include "World/MapInfra/MapManager.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <set>
#include <utility>

namespace MuEditor::MapPicker
{
namespace
{
using Data::Maps::MapRef;

// Pixel height before the editor UI scale; the picker scrolls beyond it.
constexpr float MaxPickerHeight = 420.0f;

Member StageMember(int map, int stage)
{
    return {MapRef{map, stage}, std::to_string(stage)};
}

// The levels of an event that are maps of their own, labelled 1, 2, ...
std::vector<Member> Numbered(std::initializer_list<int> maps)
{
    std::vector<Member> members;
    int level = 1;
    for (const int map : maps)
    {
        members.push_back({MapRef{map, 0}, std::to_string(level++)});
    }
    return members;
}

// The maps a server sends, by the server's numbers; the levels of an event as
// one row. The scenes of the login and character screens are left out.
std::vector<Row> MakeRows()
{
    std::vector<Row> rows;
    const auto single = [&](int map) { rows.push_back({map, {{MapRef{map, 0}, ""}}}); };
    const auto levels = [&](std::vector<Member> members)
    {
        const int first = members.front().ref.map;
        rows.push_back({first, std::move(members)});
    };
    for (const int map :
         {WD_0LORENCIA, WD_1DUNGEON, WD_2DEVIAS, WD_3NORIA, WD_4LOSTTOWER, WD_6STADIUM, WD_7ATLANSE, WD_8TARKAN})
    {
        single(map);
    }
    // The stages OpenMU sends with the map (its discriminators).
    levels({StageMember(WD_9DEVILSQUARE, 1), StageMember(WD_9DEVILSQUARE, 2), StageMember(WD_9DEVILSQUARE, 3),
            StageMember(WD_9DEVILSQUARE, 4), StageMember(SecondDevilSquareMapId, 5),
            StageMember(SecondDevilSquareMapId, 6), StageMember(SecondDevilSquareMapId, 7)});
    single(WD_10HEAVEN);
    levels(
        Numbered({WD_11BLOODCASTLE1, WD_11BLOODCASTLE1 + 1, WD_11BLOODCASTLE1 + 2, WD_11BLOODCASTLE1 + 3,
                  WD_11BLOODCASTLE1 + 4, WD_11BLOODCASTLE1 + 5, WD_11BLOODCASTLE1 + 6, WD_52BLOODCASTLE_MASTER_LEVEL}));
    levels(Numbered({WD_18CHAOS_CASTLE, WD_18CHAOS_CASTLE + 1, WD_18CHAOS_CASTLE + 2, WD_18CHAOS_CASTLE + 3,
                     WD_18CHAOS_CASTLE + 4, WD_18CHAOS_CASTLE + 5, WD_53CAOSCASTLE_MASTER_LEVEL}));
    levels(Numbered({WD_24HELLAS, WD_24HELLAS + 1, WD_24HELLAS + 2, WD_24HELLAS + 3, WD_24HELLAS + 4, WD_24HELLAS + 5,
                     WD_24HELLAS_7}));
    for (const int map :
         {WD_30BATTLECASTLE, WD_31HUNTING_GROUND, WD_33AIDA, WD_34CRYWOLF_1ST, WD_37KANTURU_1ST, WD_38KANTURU_2ND,
          WD_39KANTURU_3RD, WD_40AREA_FOR_GM, WD_41CHANGEUP3RD_1ST, WD_42CHANGEUP3RD_2ND})
    {
        single(map);
    }
    levels(Numbered({WD_45CURSEDTEMPLE_LV1, WD_45CURSEDTEMPLE_LV2, WD_45CURSEDTEMPLE_LV3, WD_45CURSEDTEMPLE_LV4,
                     WD_45CURSEDTEMPLE_LV5, WD_45CURSEDTEMPLE_LV6}));
    for (const int map : {WD_51HOME_6TH_CHAR, WD_56MAP_SWAMP_OF_QUIET, WD_57ICECITY, WD_58ICECITY_BOSS, WD_62SANTA_TOWN,
                          WD_63PK_FIELD, WD_64DUELARENA})
    {
        single(map);
    }
    levels(Numbered({WD_65DOPPLEGANGER1, WD_66DOPPLEGANGER2, WD_67DOPPLEGANGER3, WD_68DOPPLEGANGER4}));
    levels(Numbered({WD_69EMPIREGUARDIAN1, WD_70EMPIREGUARDIAN2, WD_71EMPIREGUARDIAN3, WD_72EMPIREGUARDIAN4}));
    single(WD_79UNITEDMARKETPLACE);
    levels(Numbered({WD_80KARUTAN1, WD_81KARUTAN2}));
    return rows;
}

const Row* FindRow(int map)
{
    for (const Row& row : GetRows())
    {
        for (const Member& member : row.members)
        {
            if (member.ref.map == map)
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

bool Contains(const std::vector<MapRef>& maps, const MapRef& ref)
{
    return std::find(maps.begin(), maps.end(), ref) != maps.end();
}

size_t CountChosen(const Row& row, const std::vector<MapRef>& maps)
{
    return static_cast<size_t>(std::count_if(row.members.begin(), row.members.end(),
                                             [&](const Member& member) { return IsChosen(maps, member); }));
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

bool IsKnownStage(const MapRef& ref)
{
    const std::vector<int> stages = StagesOf(ref.map);
    return std::find(stages.begin(), stages.end(), ref.stage) != stages.end();
}

// "server 11; World12", "server 9 stage 2; World10".
std::string MemberInfo(const Member& member)
{
    std::string text = "server " + std::to_string(member.ref.map);
    if (member.ref.stage != 0)
    {
        text += " stage " + std::to_string(member.ref.stage);
    }
    return text + "; World" + std::to_string(gMapManager.GetFilesFolder(member.ref.map));
}

bool RenderRow(const Row& row, std::vector<MapRef>& maps)
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
    const bool open = ImGui::TreeNodeEx("##Levels", ImGuiTreeNodeFlags_NoTreePushOnOpen);
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
        ImGui::PushID(member.ref.map * (Data::Maps::MaxStage + 1) + member.ref.stage);
        bool on = IsChosen(maps, member);
        if (ImGui::Checkbox(member.label.c_str(), &on))
        {
            SetMaps(maps, {member}, on);
            changed = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", MemberInfo(member).c_str());
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

std::vector<int> StagesOf(int map)
{
    std::vector<int> stages;
    if (const Row* row = FindRow(map))
    {
        for (const Member& member : row->members)
        {
            if (member.ref.map == map && member.ref.stage != 0)
            {
                stages.push_back(member.ref.stage);
            }
        }
    }
    return stages;
}

std::string NameOf(const MapRef& ref)
{
    const Row* row = FindRow(ref.map);
    std::string name;
    if (row == nullptr)
    {
        name = "map " + std::to_string(ref.map);
    }
    else if (IsKnownStage(ref))
    {
        return RowName(*row) + " " + std::to_string(ref.stage);
    }
    else if (const std::vector<int> stages = StagesOf(ref.map); !stages.empty())
    {
        name = RowName(*row) + " " + NumberList(stages);
    }
    else if (row->members.size() == 1)
    {
        name = RowName(*row);
    }
    else
    {
        const auto member = std::find_if(row->members.begin(), row->members.end(),
                                         [&](const Member& candidate) { return candidate.ref.map == ref.map; });
        name = RowName(*row) + " " + member->label;
    }
    // A stage the picker does not know, from hand-written data.
    return ref.stage != 0 ? name + " stage " + std::to_string(ref.stage) : name;
}

std::string Describe(const std::vector<MapRef>& maps, const char* whenEmpty)
{
    if (maps.empty())
    {
        return whenEmpty;
    }
    std::vector<MapRef> sorted = maps;
    std::sort(sorted.begin(), sorted.end());
    std::string text;
    std::vector<const Row*> wholeRows;
    for (const MapRef& ref : sorted)
    {
        const Row* row = FindRow(ref.map);
        std::string part;
        if (row != nullptr && row->members.size() > 1 && CountChosen(*row, sorted) == row->members.size())
        {
            // All levels chosen: the event's name once.
            if (std::find(wholeRows.begin(), wholeRows.end(), row) != wholeRows.end())
            {
                continue;
            }
            wholeRows.push_back(row);
            part = RowName(*row);
        }
        else
        {
            part = NameOf(ref);
        }
        text += (text.empty() ? "" : ", ") + part;
    }
    return text;
}

std::string InfoOf(const Row& row)
{
    std::vector<int> servers;
    std::set<int> folders;
    for (const Member& member : row.members)
    {
        if (std::find(servers.begin(), servers.end(), member.ref.map) == servers.end())
        {
            servers.push_back(member.ref.map);
        }
        folders.insert(gMapManager.GetFilesFolder(member.ref.map));
    }
    std::string server;
    if (StagesOf(servers.front()).empty())
    {
        server = NumberList(servers);
    }
    else
    {
        // "9 stages 1-4, 32 stages 5-7".
        for (const int map : servers)
        {
            server += (server.empty() ? "" : ", ") + std::to_string(map) + " stages " + NumberList(StagesOf(map));
        }
    }
    std::string world;
    for (const int folder : folders)
    {
        world += (world.empty() ? "World" : ", World") + std::to_string(folder);
    }
    return "server " + server + "; " + world;
}

bool IsChosen(const std::vector<MapRef>& maps, const Member& member)
{
    if (member.ref.stage == 0)
    {
        return Contains(maps, member.ref);
    }
    return Data::Maps::Names(maps, member.ref.map, member.ref.stage);
}

void SetMaps(std::vector<MapRef>& maps, const std::vector<Member>& members, bool on)
{
    for (const Member& member : members)
    {
        const int map = member.ref.map;
        const MapRef whole{map, 0};
        if (on && !IsChosen(maps, member))
        {
            maps.push_back(member.ref);
        }
        else if (!on && member.ref.stage == 0)
        {
            maps.erase(std::remove_if(maps.begin(), maps.end(), [&](const MapRef& ref) { return ref.map == map; }),
                       maps.end());
        }
        else if (!on && IsChosen(maps, member))
        {
            // The whole map gives way to its other stages.
            if (Contains(maps, whole))
            {
                maps.erase(std::remove(maps.begin(), maps.end(), whole), maps.end());
                for (const int stage : StagesOf(map))
                {
                    maps.push_back(MapRef{map, stage});
                }
            }
            maps.erase(std::remove(maps.begin(), maps.end(), member.ref), maps.end());
        }
    }
    // A map whose every stage is chosen is named without a stage.
    std::set<int> stagedMaps;
    for (const MapRef& ref : maps)
    {
        if (ref.stage != 0)
        {
            stagedMaps.insert(ref.map);
        }
    }
    for (const int map : stagedMaps)
    {
        const std::vector<int> stages = StagesOf(map);
        const bool every = !stages.empty() && std::all_of(stages.begin(), stages.end(), [&](int stage)
                                                          { return Contains(maps, MapRef{map, stage}); });
        if (every)
        {
            maps.erase(std::remove_if(maps.begin(), maps.end(), [&](const MapRef& ref) { return ref.map == map; }),
                       maps.end());
            maps.push_back(MapRef{map, 0});
        }
    }
    std::sort(maps.begin(), maps.end());
    maps.erase(std::unique(maps.begin(), maps.end()), maps.end());
}

bool Render(const char* id, std::vector<MapRef>& maps, const char* whenEmpty)
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
