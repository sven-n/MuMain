
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Core/WindowSystem.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSGiftStorageItemInfo.h"

#include "MsgBoxIGSUseItemConfirm.h"
#include "UI/Dialogs/GenericConfirmDialog.h"

void ShowIGSGiftStorageItemInfoDialog(int iStorageSeq, int iStorageItemSeq, WORD wItemCode,
                                       wchar_t szItemType, const wchar_t* pszID,
                                       const wchar_t* pszMessage, const wchar_t* pszName,
                                       const wchar_t* pszNum, const wchar_t* pszPeriod)
{
    using namespace mu::ui::window;

    wchar_t szNum[MAX_TEXT_LENGTH];
    mu_swprintf(szNum, I18N::Game::QuantityS, pszNum);
    wchar_t szPeriod[MAX_TEXT_LENGTH];
    mu_swprintf(szPeriod, I18N::Game::DurationS, pszPeriod);
    wchar_t szIDInfo[MAX_TEXT_LENGTH];
    mu_swprintf(szIDInfo, I18N::Game::ItSAGiftFromS, pszID);

    // Cash-shop items are virtual (no real level/excellent/ancient state) -- see
    // MsgBoxIGSStorageItemInfo.cpp's own comment for why this minimal snapshot matches native.
    ITEM item{};
    item.Type = wItemCode;

    GenericDialogConfig cfg;
    cfg.showCancel = true;
    cfg.title = I18N::Game::GiftInfoWindow;
    cfg.lines = { { pszName, true }, { szIDInfo, false }, { szNum, false }, { szPeriod, false }, { pszMessage, false } };
    cfg.item3D = item;
    cfg.onPrimary = [iStorageSeq, iStorageItemSeq, wItemCode, szItemType, name = std::wstring(pszName)]
    {
        ShowIGSUseItemConfirmDialog(iStorageSeq, iStorageItemSeq, wItemCode, szItemType, name.c_str());
    };
    g_pGenericConfirmDialog->Show(std::move(cfg));
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
