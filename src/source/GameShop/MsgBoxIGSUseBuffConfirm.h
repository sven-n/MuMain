#pragma once
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

// Was CMsgBoxIGSUseBuffConfirm/CMsgBoxIGSUseBuffConfirmLayout -- ported onto CGenericConfirmDialog
// (docs/rmlui-ui-system/dialog-migration-plan.md, "GameShop/MsgBoxIGS*.h"). Kept as a free function
// in this file since ShowIGSUseItemConfirmDialog (MsgBoxIGSUseItemConfirm.h) chains into it when a
// conflicting buff is already active.
void ShowIGSUseBuffConfirmDialog(int iStorageSeq, int iStorageItemSeq, WORD wItemCode,
                                  wchar_t szItemType, const wchar_t* pszItemName,
                                  const wchar_t* pszBuffName);

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
