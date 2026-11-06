#include "stdafx.h"

#include "Render/PostProcessing/MapColorGrading.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Render/PostProcessing/ColorGradingTable.h"
#include "Render/Renderer/MuRenderer.h"

#include <optional>

namespace Render::PostProcessing
{
namespace
{
std::optional<int> s_selectedMap;

void ApplyTable(const GradingTableLoad& table, int mapNumber, const std::filesystem::path& path)
{
    const auto logger = mu::log::Get("render");
    switch (table.status)
    {
    case GradingTableStatus::Missing:
        logger->info("Colour grading: map {} has no table ({}); not graded", mapNumber, path.string());
        break;
    case GradingTableStatus::Invalid:
        logger->warn("Colour grading: {} is invalid ({}); map {} not graded", path.string(), table.error, mapNumber);
        break;
    case GradingTableStatus::Loaded:
        if (mu::GetRenderer().SetColorGradingTable(table.rgba))
        {
            logger->info("Colour grading: map {} graded by {}", mapNumber, path.string());
            return;
        }
        logger->warn("Colour grading: {} could not be uploaded; map {} not graded", path.string(), mapNumber);
        break;
    }
    (void)mu::GetRenderer().SetColorGradingTable({});
}
} // namespace

void SelectMapColorGrading(int mapNumber)
{
    if (s_selectedMap == mapNumber)
    {
        return;
    }
    s_selectedMap = mapNumber;

    const std::filesystem::path path = GetGradingTablePath(mapNumber);
    ApplyTable(LoadGradingTable(path), mapNumber, path);
}

} // namespace Render::PostProcessing
