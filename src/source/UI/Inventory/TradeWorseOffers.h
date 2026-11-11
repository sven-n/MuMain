#pragma once

#include <span>

namespace UI::Trade
{
// One item the trade partner offers, held against the copies of its type the trade window remembers
// (CTrade::BackUpYourInven).
struct OfferedItemCheck
{
    int itemType = -1;
    // The remembered copy of its type is better than it.
    bool worseThanRemembered = false;
    // The remembered copy of its type is the same as it.
    bool sameAsRemembered = false;
};

// Marks the offered items the trade window warns about, in red, and returns whether any is marked, which makes the
// confirm button ask first (CTrade::AlertYourTradeInven). An item is marked when it is worse than the remembered copy
// of its type (the window remembers at most one copy per type), unless the partner also offers an item of that type
// that is the same as a remembered copy. The two spans have the same length.
[[nodiscard]] bool MarkWorseOffers(std::span<const OfferedItemCheck> offers, std::span<bool> marked);
} // namespace UI::Trade
