
#if !defined(AFX_MSGBOXIGSUSEITEMCONFIRM_H__71CD07AD_7713_4096_B88D_E06A464E39B6__INCLUDED_)
#define AFX_MSGBOXIGSUSEITEMCONFIRM_H__71CD07AD_7713_4096_B88D_E06A464E39B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

// Was CMsgBoxIGSUseItemConfirm/CMsgBoxIGSUseItemConfirmLayout -- ported onto CGenericConfirmDialog
// (docs/rmlui-ui-system/dialog-migration-plan.md, "GameShop/MsgBoxIGS*.h"). Kept as a free function
// in this file since ShowIGSStorageItemInfoDialog/ShowIGSGiftStorageItemInfoDialog both chain into
// it from their own OK button.
void ShowIGSUseItemConfirmDialog(int iStorageSeq, int iStorageItemSeq, WORD wItemCode,
                                  wchar_t szItemType, const wchar_t* pszItemName);

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#endif // !defined(AFX_MSGBOXIGSUSEITEMCONFIRM_H__71CD07AD_7713_4096_B88D_E06A464E39B6__INCLUDED_)
