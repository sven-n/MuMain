#include "stdafx.h"
#include "I18N/All.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "MsgBoxIGSUseBuffConfirm.h"

#include "Network/Server/WSclient.h" // SocketClient
#include "UI/Dialogs/GenericConfirmDialog.h"

void ShowIGSUseBuffConfirmDialog(int iStorageSeq, int iStorageItemSeq, WORD wItemCode,
                                  wchar_t szItemType, const wchar_t* pszItemName,
                                  const wchar_t* pszBuffName)
{
    using namespace mu::ui::window;

    wchar_t szDescription[256];
    mu_swprintf(szDescription, I18N::Game::UsingTheSItemWillNegate, pszItemName, pszBuffName, pszItemName);

    GenericDialogConfig cfg;
    cfg.showCancel = true;
    cfg.title = I18N::Game::BuffItemUseConfirmation;
    cfg.lines = { { szDescription, false } };
    cfg.onPrimary = [iStorageSeq, iStorageItemSeq, wItemCode, szItemType]
    {
        SocketClient->ToGameServer()->SendCashShopStorageItemConsumeRequest(iStorageSeq, iStorageItemSeq, wItemCode, szItemType);
        SocketClient->ToGameServer()->SendCashShopPointInfoRequest();
    };
    g_pGenericConfirmDialog->Show(std::move(cfg));
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
