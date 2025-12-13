// GambleSystem.cpp: implementation of the GambleSystem class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <algorithm>
#include <limits>

#include "GambleSystem.h"

namespace
{
constexpr std::int32_t kDefaultItemIndex = 0;
constexpr std::uint32_t kDefaultItemCost = 0;

bool IsValidCost(std::uint32_t cost) noexcept
{
    constexpr std::uint32_t kMaxAffordableCost = std::numeric_limits<std::uint32_t>::max();
    return cost <= kMaxAffordableCost;
}

bool IsValidItemIndex(std::int32_t index) noexcept
{
    return index >= 0;
}
} // namespace

GambleSystem& GambleSystem::Instance()
{
    static GambleSystem s_GambleSys;
    return s_GambleSys;
}

GambleSystem::GambleSystem()
{
    Init();
}

void GambleSystem::Init()
{
    m_isGambleShop = false;
    m_buyItemPosition = 0;
    m_itemInfo.ItemIndex = kDefaultItemIndex;
    m_itemInfo.ItemCost = kDefaultItemCost;
}

void GambleSystem::SetBuyItemInfo(const std::int32_t index, const std::uint32_t cost)
{
    if (IsValidItemIndex(index) && IsValidCost(cost))
    {
        m_itemInfo.ItemIndex = index;
        m_itemInfo.ItemCost = cost;
    }
    else
    {
        m_itemInfo.ItemIndex = kDefaultItemIndex;
        m_itemInfo.ItemCost = kDefaultItemCost;
    }
}
