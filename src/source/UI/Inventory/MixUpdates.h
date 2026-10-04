#pragma once

#include <cstdint>
#include <span>

// Combination results the server reports, applied to the mix (Chaos Machine and similar) and
// lucky-item windows. Item data is the item's serialized form, borrowed for the call.
namespace UI::Mix
{
// The mix window's state: Finished re-enables combining, Ready waits for the player to retry.
void SetFinished();
void SetReady();
void ClearItems();
void InsertItem(int slot, std::span<const std::uint8_t> itemData);
// An item the player moved into the mix grid: drops the picked item, then places it in range.
void PlaceMovedItem(int slot, std::span<const std::uint8_t> itemData);

// Whether the lucky-item window is waiting for a result; `requireVisible` also needs it open.
bool IsLuckyItemAwaiting(bool requireVisible);
void LuckyItemResult(bool success, int code, std::span<const std::uint8_t> itemData);
}
