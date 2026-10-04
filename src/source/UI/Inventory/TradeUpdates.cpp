#include "stdafx.h"

#include "UI/Inventory/TradeUpdates.h"

#include "UI/Core/WindowSystem.h"
#include "UI/Inventory/Trade.h"

#include <string>

namespace UI::Trade
{
void RequestReceived(std::wstring_view requester)
{
    g_pTrade->ProcessToReceiveTradeRequest(std::wstring(requester).c_str());
}

void RequestAnswered(RequestReply reply, const Partner& partner)
{
    g_pTrade->ProcessToReceiveTradeResult(reply, partner);
}

void OwnItemPlaced(int slot, std::span<const std::uint8_t> itemData)
{
    g_pTrade->ProcessToReceiveTradeItems(slot, itemData);
}

void PartnerItemAdded(int slot, std::span<const std::uint8_t> itemData)
{
    g_pTrade->ProcessToReceiveYourItemAdd(static_cast<BYTE>(slot), itemData);
}

void PartnerItemRemoved(int slot)
{
    g_pTrade->ProcessToReceiveYourItemDelete(static_cast<BYTE>(slot));
}

void OwnGoldAnswered(bool accepted)
{
    g_pTrade->ProcessToReceiveMyTradeGold(accepted ? 1 : 0);
}

void PartnerGoldChanged(int gold)
{
    g_pTrade->SetYourTradeGold(gold);
}

void PartnerConfirmChanged(PartnerConfirm state)
{
    g_pTrade->ProcessToReceiveYourConfirm(state);
}

void Closed(CloseReason reason)
{
    g_pTrade->ProcessToReceiveTradeExit(reason);
}
}
