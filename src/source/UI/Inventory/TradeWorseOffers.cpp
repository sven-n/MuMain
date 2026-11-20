#include "UI/Inventory/TradeWorseOffers.h"
#include "Engine/Object/CompareItemResult.h"

#include <algorithm>
#include <cassert>

namespace UI::Trade
{
namespace
{
bool OffersUnchangedOfType(std::span<const OfferedItemCheck> offers, int itemType)
{
    return std::ranges::any_of(offers, [itemType](const OfferedItemCheck& offer)
                               { return offer.sameAsRemembered && offer.itemType == itemType; });
}
} // namespace

OfferedItemCheck CheckOffer(int itemType, int compareWithRemembered)
{
    namespace CompareResult = Engine::Items::CompareResult;
    return {
        .itemType = itemType,
        .worseThanRemembered = (compareWithRemembered == CompareResult::FirstIsBetter),
        .sameAsRemembered = (compareWithRemembered == CompareResult::Same),
    };
}

bool MarkWorseOffers(std::span<const OfferedItemCheck> offers, std::span<bool> marked)
{
    assert(offers.size() == marked.size());
    bool anyMarked = false;
    for (size_t i = 0; i < offers.size(); ++i)
    {
        // At most 32 offers, so scanning them again per item costs less than collecting the types first.
        marked[i] = offers[i].worseThanRemembered && !OffersUnchangedOfType(offers, offers[i].itemType);
        anyMarked = anyMarked || marked[i];
    }

    return anyMarked;
}
} // namespace UI::Trade
