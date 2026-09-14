
#include "stdafx.h"
#include "I18N/All.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "MsgBoxIGSSendGiftConfirm.h"

#include "Network/Server/WSclient.h" // SocketClient
#include "UI/Dialogs/GenericConfirmDialog.h"

void ShowIGSSendGiftConfirmDialog(int iPackageSeq, int iDisplaySeq, int iPriceSeq, DWORD wItemCode,
                                   int iCashType, std::wstring id, std::wstring message,
                                   const wchar_t* pszName, const wchar_t* pszPrice,
                                   const wchar_t* pszPeriod)
{
    using namespace mu::ui::window;

    wchar_t szItemName[MAX_TEXT_LENGTH];
    wcscpy(szItemName, pszName);
    wchar_t szItemPrice[MAX_TEXT_LENGTH];
    wcscpy(szItemPrice, pszPrice);
    wchar_t szItemPeriod[MAX_TEXT_LENGTH];
    wcscpy(szItemPeriod, pszPeriod);

    GenericDialogConfig cfg;
    cfg.buttons = GenericDialogConfig::ButtonSet::OkCancel;
    cfg.title = I18N::Game::GiftConfirmation;
    cfg.lines = {
        { I18N::Game::DoYouWantToGiftTheFollowingItemS, false },
        { szItemName, true },
        { szItemPrice, true },
        { szItemPeriod, true },
        { I18N::Game::BoughtItemsUsedOrTakenOutOfStorageCannotBeReturned, false },
    };
    cfg.onPrimary = [iPackageSeq, iDisplaySeq, iPriceSeq, wItemCode, iCashType, id, message]
    {
        SocketClient->ToGameServer()->SendCashShopItemGiftRequest(
            iPackageSeq, iDisplaySeq, iPriceSeq, wItemCode, iCashType, 0,
            MU_C16(id.c_str()), MU_C16(message.c_str()));
    };
    g_pGenericConfirmDialog->Show(std::move(cfg));
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
