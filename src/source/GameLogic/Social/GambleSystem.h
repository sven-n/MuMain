// GambleSystem.h: interface for the GambleSystem class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>

struct BuyItemInfo
{
    union
    {
        struct
        {
            std::int32_t itemIndex;
            std::uint32_t itemCost;
        };
        struct
        {
            std::int32_t ItemIndex;
            std::uint32_t ItemCost;
        };
    };

    constexpr BuyItemInfo() noexcept : itemIndex(0), itemCost(0) {}
};
using BuyItemInfoPtr = BuyItemInfo*;
using LPBUYITEMINFO = BuyItemInfoPtr; // Legacy alias; prefer BuyItemInfoPtr going forward.

class GambleSystem final
{
public:
    static GambleSystem& Instance();

    GambleSystem(const GambleSystem&) = delete;
    GambleSystem& operator=(const GambleSystem&) = delete;
    GambleSystem(GambleSystem&&) = delete;
    GambleSystem& operator=(GambleSystem&&) = delete;

    ~GambleSystem() = default;

    void Init();

    void SetGambleShop(bool isGambleShop = true)
    {
        m_isGambleShop = isGambleShop;
    }
    bool IsGambleShop() const
    {
        return m_isGambleShop;
    }

    void SetBuyItemInfo(std::int32_t index, std::uint32_t cost);
    const BuyItemInfo& GetBuyItemInfoConst() const
    {
        return m_itemInfo;
    }

    void SetBuyItemPosition(std::uint8_t position)
    {
        m_buyItemPosition = position;
    }
    std::uint8_t GetBuyItemPosition() const
    {
        return m_buyItemPosition;
    }

private:
    GambleSystem();

    bool m_isGambleShop{false};
    std::uint8_t m_buyItemPosition{0};
    BuyItemInfo m_itemInfo{};
};
