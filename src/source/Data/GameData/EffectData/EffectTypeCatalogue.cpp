#include "stdafx.h"

#include "EffectTypeCatalogue.h"

#include "Data/GameData/EffectData/EffectTypeSymbols.h"

#include <algorithm>
#include <map>

namespace Data::Effects
{
EffectTypeCatalogue& EffectTypeCatalogue::GetInstance()
{
    static EffectTypeCatalogue instance;
    return instance;
}

void EffectTypeCatalogue::Build(EffectKind kind, std::span<const EffectTypeEntry> types)
{
    std::map<std::string_view, int> typeOfCode;
    for (const EffectTypeSymbol& symbol : GetEffectTypeSymbols(kind))
    {
        typeOfCode.emplace(symbol.code, symbol.type);
    }

    std::vector<NamedType> named;
    for (const EffectTypeEntry& entry : types)
    {
        const auto found = typeOfCode.find(entry.code);
        if (found != typeOfCode.end())
        {
            named.push_back({entry.name, found->second});
        }
    }

    std::vector<NamedType>& byName = m_byName[ToIndex(kind)];
    byName = named;
    std::sort(byName.begin(), byName.end(),
              [](const NamedType& left, const NamedType& right) { return left.name < right.name; });

    std::vector<NamedType>& byType = m_byType[ToIndex(kind)];
    byType = std::move(named);
    std::sort(byType.begin(), byType.end(),
              [](const NamedType& left, const NamedType& right) { return left.type < right.type; });
}

std::optional<int> EffectTypeCatalogue::FindType(EffectKind kind, std::string_view name) const
{
    const std::vector<NamedType>& byName = m_byName[ToIndex(kind)];
    const auto found = std::lower_bound(byName.begin(), byName.end(), name,
                                        [](const NamedType& named, std::string_view key) { return named.name < key; });
    if (found == byName.end() || found->name != name)
    {
        return std::nullopt;
    }
    return found->type;
}

std::string_view EffectTypeCatalogue::GetName(EffectKind kind, int type) const
{
    const std::vector<NamedType>& byType = m_byType[ToIndex(kind)];
    const auto found = std::lower_bound(byType.begin(), byType.end(), type,
                                        [](const NamedType& named, int key) { return named.type < key; });
    if (found == byType.end() || found->type != type)
    {
        return {};
    }
    return found->name;
}

size_t EffectTypeCatalogue::GetTypeCount(EffectKind kind) const
{
    return m_byName[ToIndex(kind)].size();
}
} // namespace Data::Effects
