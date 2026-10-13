#pragma once

#include <cstdint>
#include <span>
#include <string_view>

// Trade changes the server reports, applied to the trade window's own state. Item slots are the
// trade grid's; item data is the item's serialized form, borrowed for the call.
namespace UI::Trade
{
struct Partner
{
    std::wstring_view name;
    int level;
    std::uint32_t guildKey;
};

enum class RequestReply
{
    Declined,
    Unavailable,
    Accepted,
};

enum class PartnerConfirm
{
    Cleared,
    Confirmed,
    // Both confirmations clear and this player's button waits before it can be pressed again.
    BothReset,
    Unchanged,
};

enum class CloseReason
{
    Completed,
    Cancelled,
    InventoryFull,
    RequestCancelled,
    ReinforcedItem,
};

// Asks whether to trade with `requester`, or declines at once if a window that forbids trading
// is open; false when it declined.
bool RequestReceived(std::wstring_view requester);
// `partner` is used only when the request was accepted.
void RequestAnswered(RequestReply reply, const Partner& partner);

void OwnItemPlaced(int slot, std::span<const std::uint8_t> itemData);
void PartnerItemAdded(int slot, std::span<const std::uint8_t> itemData);
void PartnerItemRemoved(int slot);
void OwnGoldAnswered(bool accepted);
void PartnerGoldChanged(int gold);
void PartnerConfirmChanged(PartnerConfirm state);
void Closed(CloseReason reason);
}
