#pragma once

#include "GameLogic/Items/PersonalShopCurrency.h"
#include "UI/NewUI/Dialogs/NewUICustomMessageBox.h"

namespace SEASON3B
{
/// The input box for the price of an item of the own personal shop.
/// When the server allows other prices than zen, it offers a button
/// which switches between the allowed currencies.
class CPersonalShopPriceMsgBox : public CNewUITextInputMsgBox
{
public:
    /// Adds the currency button below the messages, if the server allows
    /// other prices than zen. Call it after the messages were added.
    void AddCurrencySelection();

    /// The selected currency, as a price without amount.
    const GameLogic::PersonalShop::ItemPrice& GetPriceType() const
    {
        return m_priceType;
    }

    bool Update() override;
    bool Render() override;

    static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

private:
    void SelectNextCurrency();

    GameLogic::PersonalShop::ItemPrice m_priceType;
    bool m_isCurrencySelectable = false;
    CNewUIMessageBoxButton m_BtnCurrency;
};
} // namespace SEASON3B

namespace UI::PersonalShop
{
/// Sends the price of an item of the own personal shop. Zen prices use the
/// original packet, so they work with every server.
void SendItemPrice(int itemSlot, const GameLogic::PersonalShop::ItemPrice& price);
} // namespace UI::PersonalShop
