#pragma once

#include <cstddef>
#include <deque>
#include <optional>
#include <utility>

namespace GameLogic::Items
{
// The item uses the hero sent to the server which wait for its answer, oldest first. The server answers them in the
// order they were sent, but its answers don't name the use: a refusal (ItemConsumptionFailed) is the oldest one, and a
// successful answer names the slot of the used item or of the item it was used on.
class PendingItemUses
{
public:
    // More than this means answers went missing; the oldest is dropped to keep the list short.
    static constexpr std::size_t MaxPending = 8;

    // `targetSlot` is the item the use is on (a jewel on a weapon), if any.
    void Add(int sourceSlot, std::optional<int> targetSlot);
    // Takes the oldest use out and gives the slot of its item: the use a refusal answers.
    std::optional<int> TakeOldest();
    // A successful answer about `slot`: the oldest use of the item there, or on it, is answered.
    void Answered(int slot);
    void Clear();
    std::size_t Count() const;

private:
    std::deque<std::pair<int, std::optional<int>>> m_uses;
};

// The hero's pending item uses.
PendingItemUses& HeroPendingItemUses();
} // namespace GameLogic::Items
