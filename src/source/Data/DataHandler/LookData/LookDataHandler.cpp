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
constexpr std::string_view ItemEffectDataLabel = "Item effect data";
constexpr std::string_view ItemEffectDataName = "item effect data";

bool LoadLooks(LookKind kind, std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    const std::filesystem::path directory = Effects::GetEffectDataDirectory();
    RenderStylesLoadResult result = LoadLookFile(directory, kind);
    const bool effects = kind == LookKind::ItemEffect;
    LogDataIssues(effects ? ItemEffectDataLabel : RenderStyleDataLabel, result.issues);
    if (Items::HasFatal(result.issues))
    {
        errorMessage =
            DescribeFatalIssues(effects ? ItemEffectDataName : RenderStyleDataName, directory, result.issues);
        return false;
    }
    const size_t count = result.looks.size();
    g_LookStore.SetLooks(kind, std::move(result.looks));
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} {} from {} in {:.1f} ms", count,
                effects ? "item effects" : "render styles", directory.string(), elapsed.count());
    return true;
}
} // namespace

RenderStylesLoadResult LoadRenderStyleFile(const std::filesystem::path& directory)
{
    return LoadLookFile(directory, LookKind::RenderStyle);
}

RenderStylesLoadResult LoadLookFile(const std::filesystem::path& directory, LookKind kind)
{
    RenderStylesLoadResult result;
    const std::filesystem::path file = directory / GetLookFileName(kind);
    const std::string source = file.generic_string();
    const std::optional<std::string> text = Items::Json::ReadTextFile(file);
    if (!text)
    {
        result.issues.push_back({Items::ItemDataIssueSeverity::Fatal, source, Items::ItemDataIssue::NoItem,
                                 Items::ItemDataIssue::NoItem, "", "could not be read"});
        return result;
    }
    LookFile read;
    ReadLookFile(*text, source, kind, read, result.issues);
    result.looks = std::move(read.looks);
    return result;
}

bool LoadRenderStyles(std::string& errorMessage)
{
    return LoadLooks(LookKind::RenderStyle, errorMessage);
}

bool LoadItemEffects(std::string& errorMessage)
{
    return LoadLooks(LookKind::ItemEffect, errorMessage);
}
} // namespace Data::Looks
