
#include "stdafx.h"
#include "I18N/All.h"

#include "Engine/Object/ZzzCharacter.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSUseItemConfirm.h"

#include "MsgBoxIGSUseBuffConfirm.h"
#include "Network/Server/WSclient.h" // SocketClient
#include "UI/Dialogs/GenericConfirmDialog.h"

void ShowIGSUseItemConfirmDialog(int iStorageSeq, int iStorageItemSeq, WORD wItemCode,
                                  wchar_t szItemType, const wchar_t* pszItemName)
{
    using namespace mu::ui::window;

    wchar_t szDescription[256];
    mu_swprintf(szDescription, I18N::Game::DoYouWishToUseS, pszItemName);

    GenericDialogConfig cfg;
    cfg.showCancel = true;
    cfg.title = I18N::Game::UseConfirmation;
    cfg.lines = { { szDescription, false } };
    cfg.onPrimary = [iStorageSeq, iStorageItemSeq, wItemCode, szItemType, itemName = std::wstring(pszItemName)]
    {
        BuffScriptLoader& pBuffInfo = TheBuffInfo();
        int iBuffType = pBuffInfo.GetBuffType(wItemCode);
        wchar_t szBuffName[MAX_TEXT_LENGTH] = { '\0', };
        bool bEqualBuff = Hero->Object.m_BuffMap.IsEqualBuffType(iBuffType, szBuffName);

#ifdef LEM_FIX_WARNINNGMSG_DELETE
        bEqualBuff = false;
#endif	// LEM_FIX_WARNINNGMSG_DELETE [lem_2010.8.18]

        if (bEqualBuff)
        {
            ShowIGSUseBuffConfirmDialog(iStorageSeq, iStorageItemSeq, wItemCode, szItemType, itemName.c_str(), szBuffName);
        }
        else
        {
            SocketClient->ToGameServer()->SendCashShopStorageItemConsumeRequest(iStorageSeq, iStorageItemSeq, wItemCode, szItemType);
            SocketClient->ToGameServer()->SendCashShopPointInfoRequest();
        }
    };
    g_pGenericConfirmDialog->Show(std::move(cfg));
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
