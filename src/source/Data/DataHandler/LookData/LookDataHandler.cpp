#include "stdafx.h"

#include "LookDataHandler.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/LookData/LookJson.h"
#include "Data/GameData/LookData/LookStore.h"

#include <chrono>

namespace Data::Looks
{
namespace
{
constexpr std::string_view RenderStyleDataLabel = "Render style data";
constexpr std::string_view RenderStyleDataName = "render style data";
} // namespace

RenderStylesLoadResult LoadRenderStyleFile(const std::filesystem::path& directory)
{
    RenderStylesLoadResult result;
    const std::filesystem::path file = directory / GetRenderStylesFileName();
    const std::string source = file.generic_string();
    const std::optional<std::string> text = Items::Json::ReadTextFile(file);
    if (!text)
    {
        result.issues.push_back({Items::ItemDataIssueSeverity::Fatal, source, Items::ItemDataIssue::NoItem,
                                 Items::ItemDataIssue::NoItem, "", "could not be read"});
        return result;
    }
    ReadRenderStylesJson(*text, source, result.looks, result.issues);
    return result;
}

bool LoadRenderStyles(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    const std::filesystem::path directory = Effects::GetEffectDataDirectory();
    RenderStylesLoadResult result = LoadRenderStyleFile(directory);
    LogDataIssues(RenderStyleDataLabel, result.issues);
    if (Items::HasFatal(result.issues))
    {
        errorMessage = DescribeFatalIssues(RenderStyleDataName, directory, result.issues);
        return false;
    }
    const size_t count = result.looks.size();
    g_LookStore.SetRenderStyles(std::move(result.looks));
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} render styles from {} in {:.1f} ms", count, directory.string(),
                elapsed.count());
    return true;
}
} // namespace Data::Looks
