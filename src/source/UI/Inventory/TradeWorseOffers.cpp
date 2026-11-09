#include "UI/Inventory/TradeWorseOffers.h"

#include <algorithm>

namespace UI::Inventory
{
namespace
{
bool OffersSameOfType(std::span<const OfferedItemCheck> offers, int itemType)
{
    return std::ranges::any_of(offers, [itemType](const OfferedItemCheck& offer)
                               { return offer.sameAsRemembered && offer.itemType == itemType; });
}
} // namespace

bool MarkWorseOffers(std::span<const OfferedItemCheck> offers, std::span<bool> marked)
{
    bool anyMarked = false;
    const size_t count = std::min(offers.size(), marked.size());
    for (size_t i = 0; i < count; ++i)
    {
        marked[i] = offers[i].worseThanRemembered && !OffersSameOfType(offers, offers[i].itemType);
        anyMarked = anyMarked || marked[i];
    }

    return anyMarked;
}
} // namespace UI::Inventory
