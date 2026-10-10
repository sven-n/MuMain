#include "stdafx.h"

#include "EffectDataHandler.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"

#include <chrono>

namespace Data::Effects
{
namespace
{
constexpr std::string_view EffectDataLabel = "Effect data";
constexpr std::string_view EffectDataName = "effect data";
} // namespace

bool LoadEffectTypes(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    const EffectTypesLoadResult result = LoadEffectTypeFiles(GetEffectDataDirectory());
    LogDataIssues(EffectDataLabel, result.issues);
    if (Items::HasErrors(result.issues))
    {
        errorMessage = DescribeDataErrors(EffectDataName, GetEffectDataDirectory(), result.issues);
        return false;
    }

    size_t typeCount = 0;
    for (const EffectKind kind : EffectKinds)
    {
        g_EffectTypeCatalogue.Build(kind, result.types[ToIndex(kind)]);
        typeCount += g_EffectTypeCatalogue.GetTypeCount(kind);
    }
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} effect type names from {} in {:.1f} ms", typeCount,
                GetEffectDataDirectory().string(), elapsed.count());
    return true;
}
} // namespace Data::Effects
