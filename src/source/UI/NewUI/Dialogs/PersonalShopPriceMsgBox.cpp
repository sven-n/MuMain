#include "stdafx.h"
#include "UI/NewUI/Dialogs/PersonalShopPriceMsgBox.h"

#include "Audio/DSPlaySound.h"
#include "I18N/All.h"
#include "Network/Server/WSclient.h"

namespace
{
// The currency button takes the height of two message lines, which are left
// blank for it below the "enter the selling price" message.
constexpr int CurrencyButtonLines = 2;
} // namespace

void SEASON3B::CPersonalShopPriceMsgBox::AddCurrencySelection()
{
    m_isCurrencySelectable = GameLogic::PersonalShop::AreOtherPricesThanZenAllowed();
    if (!m_isCurrencySelectable)
    {
        return;
    }

    for (int i = 0; i < CurrencyButtonLines; ++i)
    {
        AddMsg(L" ");
    }

    const float x = GetPos().x + (MSGBOX_WIDTH / 2) - (MSGBOX_BTN_EMPTY_WIDTH / 2);
    const float y = GetPos().y + MSGBOX_TEXT_TOP_BLANK + MSGBOX_MIDDLE_HEIGHT;
    m_BtnCurrency.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, MSGBOX_BTN_EMPTY_WIDTH,
                          MSGBOX_BTN_EMPTY_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnCurrency.SetText(GameLogic::PersonalShop::GetCurrencyName(m_priceType));

    // Replaces the callback of the base class, which is called for the other buttons.
    AddCallbackFunc(CPersonalShopPriceMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
}

bool SEASON3B::CPersonalShopPriceMsgBox::Update()
{
    if (m_isCurrencySelectable)
    {
        m_BtnCurrency.Update();
    }

    return CNewUITextInputMsgBox::Update();
}

bool SEASON3B::CPersonalShopPriceMsgBox::Render()
{
    CNewUITextInputMsgBox::Render();
    if (!m_isCurrencySelectable)
    {
        return true;
    }

    EnableAlphaTest();
    m_BtnCurrency.Render();
    DisableAlphaBlend();
    return true;
}

SEASON3B::CALLBACK_RESULT SEASON3B::CPersonalShopPriceMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner,
                                                                        const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CPersonalShopPriceMsgBox*>(pOwner);
    if (pMsgBox && pMsgBox->m_isCurrencySelectable && pMsgBox->m_BtnCurrency.IsMouseIn())
    {
        pMsgBox->SelectNextCurrency();
        PlayBuffer(SOUND_CLICK01);
        return CALLBACK_BREAK;
    }

    return CNewUITextInputMsgBox::LButtonUp(pOwner, xParam);
}

void SEASON3B::CPersonalShopPriceMsgBox::SelectNextCurrency()
{
    m_priceType = GameLogic::PersonalShop::GetNextAllowedPriceType(m_priceType);
    m_BtnCurrency.SetText(GameLogic::PersonalShop::GetCurrencyName(m_priceType));
}

namespace UI::PersonalShop
{
void SendItemPrice(int itemSlot, const GameLogic::PersonalShop::ItemPrice& price)
{
    if (price.Kind == GameLogic::PersonalShop::PriceKind::Zen)
    {
        SocketClient->ToGameServer()->SendPlayerShopSetItemPrice(itemSlot, price.Amount);
        return;
    }

    const auto encoded = GameLogic::PersonalShop::EncodeExtendedPrice(price);
    SocketClient->ToGameServer()->SendPlayerShopSetItemPriceExtended(encoded.MoneyPrice, encoded.PriceItemType,
                                                                     encoded.RequiredItemAmount, itemSlot);
}
} // namespace UI::PersonalShop
