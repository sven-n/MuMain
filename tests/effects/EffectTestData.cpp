#include "stdafx.h"

#include "EffectTestData.h"

#include "doctest.h"

#include "Data/GameData/EffectData/EffectKind.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"
#include "Render/Effects/EffectRegistry.h"

#include <algorithm>
#include <vector>

namespace EffectTestData
{
namespace
{
bool Contains(std::span<const int> types, int type)
{
    return std::find(types.begin(), types.end(), type) != types.end();
}

bool HasRowOf(std::span<const Data::Effects::EffectTypeCreateParams> rows, int type)
{
    return std::any_of(rows.begin(), rows.end(),
                       [&](const Data::Effects::EffectTypeCreateParams& row) { return row.type == type; });
}
} // namespace

std::filesystem::path ShippedEffectDirectory()
{
    return std::filesystem::path(MU_TEST_DATA_DIR) / "Effects";
}

const Data::Effects::EffectTypesLoadResult& ShippedTypes()
{
    static const Data::Effects::EffectTypesLoadResult result =
        Data::Effects::LoadEffectTypeFiles(ShippedEffectDirectory());
    return result;
}

void BuildShippedRegistry(std::span<const int> withoutCreationValuesOf,
                          std::span<const Data::Effects::EffectTypeCreateParams> extraRows)
{
    Data::Effects::EffectTypeCatalogue catalogue;
    catalogue.Build(Data::Effects::EffectKind::Effect,
                    ShippedTypes().types[Data::Effects::ToIndex(Data::Effects::EffectKind::Effect)]);
    const auto shippedRows = catalogue.GetCreateParams();
    for (const int type : withoutCreationValuesOf)
    {
        REQUIRE_MESSAGE(HasRowOf(shippedRows, type), "type " << type << " has no creation values in the catalogue");
    }

    std::vector<Data::Effects::EffectTypeCreateParams> rows;
    for (const Data::Effects::EffectTypeCreateParams& row : shippedRows)
    {
        if (!Contains(withoutCreationValuesOf, row.type) && !HasRowOf(extraRows, row.type))
        {
            rows.push_back(row);
        }
    }
    rows.insert(rows.end(), extraRows.begin(), extraRows.end());
    Render::Effects::BuildRegistry(rows);
}
} // namespace EffectTestData
