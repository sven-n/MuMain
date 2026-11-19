#pragma once

#include "Data/DataHandler/LookData/LookDataHandler.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookStore.h"

#include <filesystem>
#include <vector>

// The shipped looks (Data/Effects/RenderStyles.json and ItemEffects.json) in the game's look store for as long as it
// lives, and the looks from before again afterwards: the item looks that are data draw only with them.
class ShippedLooksScope
{
public:
    ShippedLooksScope()
        : m_renderStyles(Data::Looks::g_LookStore.GetRenderStyles().begin(),
                         Data::Looks::g_LookStore.GetRenderStyles().end()),
          m_itemEffects(Data::Looks::g_LookStore.GetItemEffects().begin(),
                        Data::Looks::g_LookStore.GetItemEffects().end())
    {
        const std::filesystem::path effects = std::filesystem::path(MU_TEST_DATA_DIR) / "Effects";
        for (const Data::Looks::LookKind kind : {Data::Looks::LookKind::RenderStyle, Data::Looks::LookKind::ItemEffect})
        {
            Data::Looks::RenderStylesLoadResult shipped = Data::Looks::LoadLookFile(effects, kind);
            m_complete = m_complete && !Data::Items::HasFatal(shipped.issues);
            Data::Looks::g_LookStore.SetLooks(kind, std::move(shipped.looks));
        }
    }

    ~ShippedLooksScope()
    {
        Data::Looks::g_LookStore.SetRenderStyles(std::move(m_renderStyles));
        Data::Looks::g_LookStore.SetItemEffects(std::move(m_itemEffects));
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
    std::vector<Data::Looks::LookDefinition> m_itemEffects;
    bool m_complete = true;
};
