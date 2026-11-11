
#if !defined(AFX_MSGBOXIGSGIFTSTORAGEITEMINFO_H__C2B5CD4A_A0D5_4F4D_854B_C9119EE0EED6__INCLUDED_)
#define AFX_MSGBOXIGSGIFTSTORAGEITEMINFO_H__C2B5CD4A_A0D5_4F4D_854B_C9119EE0EED6__INCLUDED_

#pragma once

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

// Was CMsgBoxIGSGiftStorageItemInfo/CMsgBoxIGSGiftStorageItemInfoLayout -- ported onto
// CGenericConfirmDialog (docs/rmlui-ui-system/dialog-migration-plan.md, "GameShop/MsgBoxIGS*.h").
// Native's own m_MessageInputBox is read-only display here (SetText in Initialize, never read
// back), so the gift message is just one more plain body line -- no `input` field needed.
void ShowIGSGiftStorageItemInfoDialog(int iStorageSeq, int iStorageItemSeq, WORD wItemCode,
                                       wchar_t szItemType, const wchar_t* pszID,
                                       const wchar_t* pszMessage, const wchar_t* pszName,
                                       const wchar_t* pszNum, const wchar_t* pszPeriod);

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#endif // !defined(AFX_MSGBOXIGSGIFTSTORAGEITEMINFO_H__C2B5CD4A_A0D5_4F4D_854B_C9119EE0EED6__INCLUDED_)
