
#include "stdafx.h"
#include "I18N/All.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "MsgBoxIGSBuyConfirm.h"

#include "Network/Server/WSclient.h" // SocketClient
#include "UI/Dialogs/GenericConfirmDialog.h"

void ShowIGSBuyConfirmDialog(int iPackageSeq, int iDisplaySeq, int iPriceSeq, WORD wItemCode,
                             int iCashType, const wchar_t* pszName, const wchar_t* pszPrice,
                             const wchar_t* pszPeriod)
{
    using namespace mu::ui::window;

    wchar_t szItemName[MAX_TEXT_LENGTH];
    wchar_t szItemPrice[MAX_TEXT_LENGTH];
    wchar_t szItemPeriod[MAX_TEXT_LENGTH];
    mu_swprintf(szItemName, I18N::Game::ItemS, pszName);
    mu_swprintf(szItemPrice, I18N::Game::PriceS, pszPrice);
    mu_swprintf(szItemPeriod, I18N::Game::DurationS, pszPeriod);

    GenericDialogConfig cfg;
    cfg.buttons = GenericDialogConfig::ButtonSet::OkCancel;
    cfg.title = I18N::Game::PurchaseConfirmation;
    cfg.lines = {
        { I18N::Game::DoYouWishToBuyTheFollowingItemS, false },
        { szItemName, true },
        { szItemPrice, true },
        { szItemPeriod, true },
        { I18N::Game::BoughtItemsUsedOrTakenOutOfStorageCannotBeReturned, false },
    };
    cfg.onPrimary = [iPackageSeq, iDisplaySeq, iPriceSeq, wItemCode, iCashType]
    {
        SocketClient->ToGameServer()->SendCashShopItemBuyRequest(iPackageSeq, iDisplaySeq, iPriceSeq,
                                                                  wItemCode, iCashType, 0);
    };
    g_pGenericConfirmDialog->Show(std::move(cfg));
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
