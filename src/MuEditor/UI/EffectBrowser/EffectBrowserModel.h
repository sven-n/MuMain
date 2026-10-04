#pragma once

#ifdef _EDITOR

#include "EffectCreateTable.h"
#include "EffectStages.h"
#include "EffectTypeAsset.h"

#include "Data/GameData/EffectData/EffectCreateParams.h"
#include "Data/GameData/EffectData/EffectKind.h"

#include <array>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Effects
{
class EffectTypeCatalogue;
}

namespace MuEditor::Effects
{
struct EffectTypeRef
{
    Data::Effects::EffectKind kind = Data::Effects::EffectKind::Effect;
    int type = 0;

    bool operator==(const EffectTypeRef&) const = default;
};

// A type as the effect browser lists it.
struct EffectBrowserRow
{
    int type = 0;
    std::string name;
    std::string_view code;
    // The name, the code and the number in lowercase, for the search.
    std::string searchText;
    // Effects only: particles, joints and sprites have no registry.
    EffectStages stages;
    EffectAssetSlot assetSlot = EffectAssetSlot::Texture;
    EffectAsset asset;
};

// A text as the search compares it: lowercase.
std::string ToSearchText(std::string_view text);

struct EffectBrowserFilter
{
    // A part of the name, the code or the number, as ToSearchText makes it.
    std::string search;
    // Effects only; nullopt for any stage. OnGround also lists the effects
    // that RenderEffectShadows draws besides another stage.
    std::optional<CreateStage> create;
    std::optional<MoveStage> move;
    std::optional<RenderStage> render;
    // Types whose slot holds something right now.
    bool onlyLoaded = false;

    bool operator==(const EffectBrowserFilter&) const = default;
};

// What the details of a type show besides its row.
struct EffectBrowserDetails
{
    // The types of the other kinds with the same number.
    std::vector<EffectTypeRef> sameNumber;
    // The other effect types with the same move handler or creation hook.
    std::vector<int> sameMoveHandler;
    std::vector<int> sameCreateHook;
    // Effects with a registry row.
    std::optional<EffectCreateTable> creation;
};

// The types of every kind with what the effect browser shows of them, built
// once when the catalogue and the registry are loaded. Needs no ImGui, so the
// tests check it.
class EffectBrowserModel
{
public:
    using DescriptorLookup = const Render::Effects::EffectDescriptor* (*)(int type);

    // Lists the types of every kind with the names of `catalogue`, and the
    // stages of the effects from the registry (`lookup`) and the list of the
    // legacy cases.
    void Build(const Data::Effects::EffectTypeCatalogue& catalogue, DescriptorLookup lookup);
    bool IsBuilt() const
    {
        return m_built;
    }

    // Reads again what the slot of each type holds.
    void RefreshAssets(const EffectAssetProbe& probe);
    // Changes with each RefreshAssets.
    int GetAssetGeneration() const
    {
        return m_assetGeneration;
    }

    // Sorted by number.
    std::span<const EffectBrowserRow> GetRows(Data::Effects::EffectKind kind) const;
    const EffectBrowserRow* FindRow(Data::Effects::EffectKind kind, int type) const;

    // The indexes in GetRows(kind) of the rows the filter lists.
    std::vector<int> Filter(Data::Effects::EffectKind kind, const EffectBrowserFilter& filter) const;
    static bool IsListed(Data::Effects::EffectKind kind, const EffectBrowserRow& row,
                         const EffectBrowserFilter& filter);

    EffectBrowserDetails Describe(EffectTypeRef ref) const;

private:
    void BuildRows(Data::Effects::EffectKind kind, const Data::Effects::EffectTypeCatalogue& catalogue,
                   DescriptorLookup lookup);
    void GroupSharedCode(DescriptorLookup lookup);
    std::optional<EffectCreateTable> DescribeCreation(int type) const;

    std::array<std::vector<EffectBrowserRow>, Data::Effects::EffectKindCount> m_rows;
    std::vector<Data::Effects::EffectTypeCreateParams> m_createParams;
    // The effect types that share a move handler or a creation hook, a list
    // per handler that more than one type has.
    std::vector<std::vector<int>> m_moveHandlerGroups;
    std::vector<std::vector<int>> m_createHookGroups;
    int m_assetGeneration = 0;
    bool m_built = false;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
