#include "stdafx.h"

#ifdef _EDITOR

#include "EffectBrowserModel.h"

#include "EffectLegacyCases.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"
#include "Data/GameData/EffectData/EffectTypeSymbols.h"
#include "Render/Effects/EffectDef.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <map>
#include <utility>

namespace MuEditor::Effects
{
using Data::Effects::EffectKind;

namespace
{
std::string SearchTextOf(const EffectBrowserRow& row)
{
    return ToSearchText(row.name) + ' ' + ToSearchText(row.code) + ' ' + std::to_string(row.type);
}

bool MatchesRenderFilter(const EffectStages& stages, RenderStage filter)
{
    return stages.render == filter || (filter == RenderStage::OnGround && stages.drawnOnGround);
}

template <typename Handler>
std::vector<std::vector<int>> GroupsOfMoreThanOne(const std::map<Handler, std::vector<int>>& typesByHandler)
{
    std::vector<std::vector<int>> groups;
    for (const auto& [handler, types] : typesByHandler)
    {
        if (types.size() > 1)
            groups.push_back(types);
    }
    return groups;
}

// The other types of the group that has `type`.
std::vector<int> OthersInGroup(const std::vector<std::vector<int>>& groups, int type)
{
    for (const std::vector<int>& group : groups)
    {
        if (std::find(group.begin(), group.end(), type) == group.end())
            continue;
        std::vector<int> others;
        std::copy_if(group.begin(), group.end(), std::back_inserter(others),
                     [type](int other) { return other != type; });
        return others;
    }
    return {};
}
} // namespace

std::string ToSearchText(std::string_view text)
{
    std::string lower(text);
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

void EffectBrowserModel::Build(const Data::Effects::EffectTypeCatalogue& catalogue, DescriptorLookup lookup)
{
    for (const EffectKind kind : Data::Effects::EffectKinds)
    {
        BuildRows(kind, catalogue, lookup);
    }
    const std::span<const Data::Effects::EffectTypeCreateParams> createParams = catalogue.GetCreateParams();
    m_createParams.assign(createParams.begin(), createParams.end());
    GroupSharedCode(lookup);
    m_built = true;
}

void EffectBrowserModel::BuildRows(EffectKind kind, const Data::Effects::EffectTypeCatalogue& catalogue,
                                   DescriptorLookup lookup)
{
    std::vector<EffectBrowserRow>& rows = m_rows[Data::Effects::ToIndex(kind)];
    rows.clear();
    for (const Data::Effects::EffectTypeSymbol& symbol : Data::Effects::GetEffectTypeSymbols(kind))
    {
        EffectBrowserRow row;
        row.type = symbol.type;
        row.name = catalogue.GetName(kind, symbol.type);
        row.code = symbol.code;
        row.searchText = SearchTextOf(row);
        if (kind == EffectKind::Effect)
        {
            row.stages = DescribeEffectStages(symbol.type, lookup(symbol.type), FindEffectLegacyCases(symbol.type));
        }
        row.assetSlot = GetAssetSlot(kind, symbol.type);
        rows.push_back(std::move(row));
    }
}

void EffectBrowserModel::GroupSharedCode(DescriptorLookup lookup)
{
    std::map<Render::Effects::MoveHandler, std::vector<int>> typesByMoveHandler;
    std::map<Render::Effects::CreateHook, std::vector<int>> typesByCreateHook;
    for (const EffectBrowserRow& row : GetRows(EffectKind::Effect))
    {
        const Render::Effects::EffectDescriptor* descriptor = lookup(row.type);
        if (descriptor == nullptr)
            continue;
        if (descriptor->move != nullptr)
            typesByMoveHandler[descriptor->move].push_back(row.type);
        if (descriptor->onCreate != nullptr)
            typesByCreateHook[descriptor->onCreate].push_back(row.type);
    }
    m_moveHandlerGroups = GroupsOfMoreThanOne(typesByMoveHandler);
    m_createHookGroups = GroupsOfMoreThanOne(typesByCreateHook);
}

void EffectBrowserModel::RefreshAssets(const EffectAssetProbe& probe)
{
    for (std::vector<EffectBrowserRow>& rows : m_rows)
    {
        for (EffectBrowserRow& row : rows)
        {
            row.asset = probe(row.assetSlot, row.type);
        }
    }
    ++m_assetGeneration;
}

std::span<const EffectBrowserRow> EffectBrowserModel::GetRows(EffectKind kind) const
{
    return m_rows[Data::Effects::ToIndex(kind)];
}

const EffectBrowserRow* EffectBrowserModel::FindRow(EffectKind kind, int type) const
{
    const std::span<const EffectBrowserRow> rows = GetRows(kind);
    const auto found = std::lower_bound(rows.begin(), rows.end(), type,
                                        [](const EffectBrowserRow& row, int value) { return row.type < value; });
    return found != rows.end() && found->type == type ? &*found : nullptr;
}

std::vector<int> EffectBrowserModel::Filter(EffectKind kind, const EffectBrowserFilter& filter) const
{
    std::vector<int> listed;
    const std::span<const EffectBrowserRow> rows = GetRows(kind);
    for (size_t i = 0; i < rows.size(); ++i)
    {
        if (IsListed(kind, rows[i], filter))
            listed.push_back(static_cast<int>(i));
    }
    return listed;
}

bool EffectBrowserModel::IsListed(EffectKind kind, const EffectBrowserRow& row, const EffectBrowserFilter& filter)
{
    if (!filter.search.empty() && row.searchText.find(filter.search) == std::string::npos)
        return false;
    if (filter.onlyLoaded && !row.asset.loaded)
        return false;
    if (kind != EffectKind::Effect)
        return true;
    return (!filter.create || row.stages.create == *filter.create) &&
           (!filter.move || row.stages.move == *filter.move) &&
           (!filter.render || MatchesRenderFilter(row.stages, *filter.render));
}

EffectBrowserDetails EffectBrowserModel::Describe(EffectTypeRef ref) const
{
    EffectBrowserDetails details;
    for (const EffectKind other : Data::Effects::EffectKinds)
    {
        if (other != ref.kind && FindRow(other, ref.type) != nullptr)
            details.sameNumber.push_back({other, ref.type});
    }
    if (ref.kind != EffectKind::Effect)
        return details;
    details.sameMoveHandler = OthersInGroup(m_moveHandlerGroups, ref.type);
    details.sameCreateHook = OthersInGroup(m_createHookGroups, ref.type);
    details.creation = DescribeCreation(ref.type);
    return details;
}

std::optional<EffectCreateTable> EffectBrowserModel::DescribeCreation(int type) const
{
    const auto found =
        std::lower_bound(m_createParams.begin(), m_createParams.end(), type,
                         [](const Data::Effects::EffectTypeCreateParams& row, int value) { return row.type < value; });
    if (found == m_createParams.end() || found->type != type)
        return std::nullopt;
    return BuildEffectCreateTable(found->params);
}
} // namespace MuEditor::Effects

#endif // _EDITOR
