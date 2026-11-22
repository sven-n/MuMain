#include "stdafx.h"

#include "WarpStorage.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/MapData/WarpDatabase.h"
#include "Data/GameData/MapData/WarpsJson.h"

#include <chrono>

namespace Data::Maps
{
namespace
{
constexpr const char* WarpsFile = "Data/Maps/Warps.json";
constexpr std::string_view WarpsLabel = "Warps";
constexpr std::string_view WarpsName = "warp list";
} // namespace

std::filesystem::path GetWarpsFile()
{
    return WarpsFile;
}

WarpsLoadResult LoadWarpsFile(const std::filesystem::path& file)
{
    WarpsLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        result.issues.push_back({Items::ItemDataIssueSeverity::Fatal, source, Items::ItemDataIssue::NoItem,
                                 Items::ItemDataIssue::NoItem, "", "could not be read"});
        return result;
    }
    ReadWarpsJson(*text, source, result.list, result.issues);
    ValidateWarps(result.list, source, result.issues);
    return result;
}

std::optional<std::string> MakeWarpsFileText(const std::filesystem::path& file, const WarpList& list,
                                             std::vector<Items::ItemDataIssue>& issues)
{
    const std::string source = file.generic_string();
    std::string text = WriteWarpsJson(list);
    WarpList readBack;
    std::vector<Items::ItemDataIssue> found;
    ReadWarpsJson(text, source, readBack, found);
    ValidateWarps(readBack, source, found);
    issues.insert(issues.end(), found.begin(), found.end());
    if (Items::HasFatal(found))
    {
        return std::nullopt;
    }
    return text;
}

void ValidateWarpItemsInGame(const WarpList& list, const std::string& source, std::vector<Items::ItemDataIssue>& issues)
{
    ValidateWarpItems(
        list, [](int group, int number) { return g_ItemDatabase.FindEntry(group, number) != nullptr; }, source, issues);
}

bool LoadWarps(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    WarpsLoadResult result = LoadWarpsFile(GetWarpsFile());
    if (!Items::HasFatal(result.issues))
    {
        ValidateWarpItemsInGame(result.list, GetWarpsFile().generic_string(), result.issues);
    }
    LogDataIssues(WarpsLabel, result.issues);
    if (Items::HasFatal(result.issues))
    {
        errorMessage = DescribeFatalIssues(WarpsName, GetWarpsFile(), result.issues);
        return false;
    }
    g_WarpDatabase.Build(result.list);
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} warps from {} in {:.1f} ms", result.list.warps.size(),
                GetWarpsFile().generic_string(), elapsed.count());
    return true;
}

bool SaveWarps(std::vector<Items::ItemDataIssue>& issues)
{
    const std::optional<std::string> text = MakeWarpsFileText(GetWarpsFile(), g_WarpDatabase.GetList(), issues);
    if (!text)
    {
        return false;
    }
    std::string error;
    if (!WriteTextFile(GetWarpsFile(), *text, error))
    {
        issues.push_back({Items::ItemDataIssueSeverity::Fatal, GetWarpsFile().generic_string(),
                          Items::ItemDataIssue::NoItem, Items::ItemDataIssue::NoItem, "", error});
        return false;
    }
    return true;
}

bool RevertWarps(std::vector<Items::ItemDataIssue>& issues)
{
    WarpsLoadResult result = LoadWarpsFile(GetWarpsFile());
    issues.insert(issues.end(), result.issues.begin(), result.issues.end());
    if (Items::HasFatal(result.issues))
    {
        return false;
    }
    g_WarpDatabase.Build(result.list);
    return true;
}
} // namespace Data::Maps
