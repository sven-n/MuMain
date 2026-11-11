#include "UI/Inventory/TradeWorseOffers.h"

#include <algorithm>
#include <vector>

namespace UI::Trade
{
namespace
{
std::vector<int> TypesOfferedUnchanged(std::span<const OfferedItemCheck> offers)
{
    std::vector<int> types;
    for (const OfferedItemCheck& offer : offers)
    {
        if (offer.sameAsRemembered)
            types.push_back(offer.itemType);
    }

    return types;
}
} // namespace

bool MarkWorseOffers(std::span<const OfferedItemCheck> offers, std::span<bool> marked)
{
    const std::vector<int> unchangedTypes = TypesOfferedUnchanged(offers);
    bool anyMarked = false;
    const size_t count = std::min(offers.size(), marked.size());
    for (size_t i = 0; i < count; ++i)
    {
        marked[i] = offers[i].worseThanRemembered &&
                    std::ranges::find(unchangedTypes, offers[i].itemType) == unchangedTypes.end();
        anyMarked = anyMarked || marked[i];
    }

    return anyMarked;
}
} // namespace UI::Trade
