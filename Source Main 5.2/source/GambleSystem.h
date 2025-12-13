// GambleSystem.h: interface for the GambleSystem class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>

struct BUYITEMINFO
{
    std::int32_t ItemIndex{0};
    std::uint32_t ItemCost{0};
};
using LPBUYITEMINFO = BUYITEMINFO*;

class GambleSystem final
{
public:
    using BuyItemInfo = BUYITEMINFO;

    static GambleSystem& Instance();

    GambleSystem(const GambleSystem&) = delete;
    GambleSystem& operator=(const GambleSystem&) = delete;
    GambleSystem(GambleSystem&&) = delete;
    GambleSystem& operator=(GambleSystem&&) = delete;

    ~GambleSystem() = default;

    void Init();

    void SetGambleShop(bool isGambleShop = true) { m_isGambleShop = isGambleShop; }
    bool IsGambleShop() const { return m_isGambleShop; }

    void SetBuyItemInfo(std::int32_t index, std::uint32_t cost);
    LPBUYITEMINFO GetBuyItemInfo() { return &m_itemInfo; }
    const BuyItemInfo& GetBuyItemInfoConst() const { return m_itemInfo; }

    void SetBuyItemPosition(std::uint8_t position) { m_buyItemPosition = position; }
    std::uint8_t GetBuyItemPosition() const { return m_buyItemPosition; }

private:
    GambleSystem();

    bool m_isGambleShop{false};
    std::uint8_t m_buyItemPosition{0};
    BuyItemInfo m_itemInfo{};
};
