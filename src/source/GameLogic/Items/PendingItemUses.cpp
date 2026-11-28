#include "stdafx.h"
#include "GameLogic/Items/PendingItemUses.h"

#include <algorithm>

namespace GameLogic::Items
{
void PendingItemUses::Add(int sourceSlot, std::optional<int> targetSlot)
{
    if (m_uses.size() >= MaxPending)
    {
        m_uses.pop_front();
    }

    m_uses.emplace_back(sourceSlot, targetSlot);
}

std::optional<int> PendingItemUses::TakeOldest()
{
    if (m_uses.empty())
    {
        return std::nullopt;
    }

    const int sourceSlot = m_uses.front().first;
    m_uses.pop_front();
    return sourceSlot;
}

void PendingItemUses::Answered(int slot)
{
    const auto answered =
        std::find_if(m_uses.begin(), m_uses.end(), [slot](const std::pair<int, std::optional<int>>& use)
                     { return use.first == slot || use.second == slot; });
    if (answered != m_uses.end())
    {
        m_uses.erase(answered);
    }
}

void PendingItemUses::Clear()
{
    m_uses.clear();
}

std::size_t PendingItemUses::Count() const
{
    return m_uses.size();
}

PendingItemUses& HeroPendingItemUses()
{
    static PendingItemUses uses;
    return uses;
}
} // namespace GameLogic::Items
