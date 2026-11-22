#pragma once

#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/DataHandler/LookData/LookDataHandler.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookStore.h"

#include <filesystem>
#include <vector>

// The shipped looks (Data/Effects/RenderStyles.json and Effects.json) in the game's look store for as long as it
// lives, and the looks from before again afterwards: the item looks that are data draw only with them. The effect
// catalogue, which things on bones name their types in, is loaded too while it lives, unless it was.
class ShippedLooksScope
{
public:
    ShippedLooksScope()
        : m_renderStyles(Data::Looks::g_LookStore.GetRenderStyles().begin(),
                         Data::Looks::g_LookStore.GetRenderStyles().end()),
          m_effects(Data::Looks::g_LookStore.GetEffects().begin(),
                        Data::Looks::g_LookStore.GetEffects().end())
    {
        const std::filesystem::path effects = std::filesystem::path(MU_TEST_DATA_DIR) / "Effects";
        for (const Data::Looks::LookKind kind : {Data::Looks::LookKind::RenderStyle, Data::Looks::LookKind::Effect})
        {
            Data::Looks::RenderStylesLoadResult shipped = Data::Looks::LoadLookFile(effects, kind);
            m_complete = m_complete && !Data::Items::HasFatal(shipped.issues);
            Data::Looks::g_LookStore.SetLooks(kind, std::move(shipped.looks));
        }
        m_builtCatalogue = g_EffectTypeCatalogue.GetTypeCount(Data::Effects::EffectKind::Sprite) == 0;
        if (m_builtCatalogue)
        {
            const Data::Effects::EffectTypesLoadResult types = Data::Effects::LoadEffectTypeFiles(effects);
            m_complete = m_complete && !Data::Items::HasFatal(types.issues);
            for (const Data::Effects::EffectKind kind : Data::Effects::EffectKinds)
            {
                g_EffectTypeCatalogue.Build(kind, types.types[Data::Effects::ToIndex(kind)]);
            }
        }
    }

    ~ShippedLooksScope()
    {
        if (m_builtCatalogue)
        {
            for (const Data::Effects::EffectKind kind : Data::Effects::EffectKinds)
            {
                g_EffectTypeCatalogue.Build(kind, {});
            }
        }
        Data::Looks::g_LookStore.SetRenderStyles(std::move(m_renderStyles));
        Data::Looks::g_LookStore.SetEffects(std::move(m_effects));
    }

    ShippedLooksScope(const ShippedLooksScope&) = delete;
    ShippedLooksScope& operator=(const ShippedLooksScope&) = delete;

    // Whether both files were read without a fatal problem.
    [[nodiscard]] bool IsComplete() const
    {
        return m_complete;
    }

private:
    std::vector<Data::Looks::LookDefinition> m_renderStyles;
    std::vector<Data::Looks::LookDefinition> m_effects;
    bool m_complete = true;
    bool m_builtCatalogue = false;
};
