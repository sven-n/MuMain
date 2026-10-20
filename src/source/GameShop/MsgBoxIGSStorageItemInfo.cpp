
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Core/WindowSystem.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "MsgBoxIGSStorageItemInfo.h"

#include "MsgBoxIGSUseItemConfirm.h"
#include "UI/Dialogs/GenericConfirmDialog.h"

void ShowIGSStorageItemInfoDialog(int iStorageSeq, int iStorageItemSeq, WORD wItemCode,
                                   char szItemType, const wchar_t* pszName, const wchar_t* pszNum,
                                   const wchar_t* pszPeriod)
{
    using namespace mu::ui::window;

    wchar_t szNum[MAX_TEXT_LENGTH];
    mu_swprintf(szNum, I18N::Game::QuantityS, pszNum);
    wchar_t szPeriod[MAX_TEXT_LENGTH];
    mu_swprintf(szPeriod, I18N::Game::DurationS, pszPeriod);

    // Cash-shop items are virtual (no real level/excellent/ancient state) -- native's own
    // RenderItem3D(..., wItemCode, 0, 0, 0, true) call passes zeros for those, so a minimal
    // snapshot with just Type set reproduces it exactly (see GenericConfirmDialog.cpp's Render3D()).
    ITEM item{};
    item.Type = wItemCode;

    GenericDialogConfig cfg;
    cfg.showCancel = true;
    cfg.title = I18N::Game::ItemInfoWindow;
    cfg.lines = { { pszName, true }, { szNum, false }, { szPeriod, false } };
    cfg.item3D = item;
    cfg.onPrimary = [iStorageSeq, iStorageItemSeq, wItemCode, szItemType, name = std::wstring(pszName)]
    {
        ShowIGSUseItemConfirmDialog(iStorageSeq, iStorageItemSeq, wItemCode, szItemType, name.c_str());
    };
    g_pGenericConfirmDialog->Show(std::move(cfg));
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
