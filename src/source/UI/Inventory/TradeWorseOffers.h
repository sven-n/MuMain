#pragma once

#include "Engine/Object/CompareItemResult.h"

#include <algorithm>
#include <span>

namespace UI::Trade
{
// One item the trade partner offers, held against the copy of its type the trade window remembers
// (CTrade::BackUpYourInven).
struct OfferedItemCheck
{
    int itemType = -1;
    // The remembered copy of its type is better than it, or has another additional option kind
    // (CompareResult::OtherOptionKind), so the swap cannot be ranked and is warned about.
    bool worseThanRemembered = false;
    // The remembered copy of its type is the same as it.
    bool sameAsRemembered = false;
};

// Reads CompareItem(remembered copy, offered item) for an offered item of `itemType`. Pass
// Engine::Items::CompareResult::OtherType when no copy of its type is remembered.
[[nodiscard]] OfferedItemCheck CheckOffer(int itemType, int compareWithRemembered);

// Holds `offered` against the copy of its type in `remembered` (the window remembers at most one copy per type,
// CTrade::BackUpYourInven). `compare` is CompareItem: it gets the remembered copy first and the offered item second.
template <typename Item, typename Compare>
[[nodiscard]] OfferedItemCheck CheckAgainstRemembered(std::span<const Item> remembered, const Item& offered,
                                                      Compare compare)
{
    const auto copy =
        std::ranges::find_if(remembered, [&offered](const Item& item) { return item.Type == offered.Type; });
    const int compareResult =
        (copy == remembered.end()) ? Engine::Items::CompareResult::OtherType : compare(*copy, offered);
    return CheckOffer(offered.Type, compareResult);
}

// Marks the offered items the trade window warns about, in red, and returns whether any is marked, which makes the
// confirm button ask first (CTrade::AlertYourTradeInven). An item is marked when it is worse than the remembered copy
// of its type (the window remembers at most one copy per type), unless the partner also offers an item of that type
// that is the same as the remembered copy. `marked` must have the length of `offers`.
[[nodiscard]] bool MarkWorseOffers(std::span<const OfferedItemCheck> offers, std::span<bool> marked);
} // namespace UI::Trade
