
#if !defined(AFX_MSGBOXIGSUSEITEM_H__5E717B05_9D6D_4E85_B168_47D5EEA59CF7__INCLUDED_)
#define AFX_MSGBOXIGSUSEITEM_H__5E717B05_9D6D_4E85_B168_47D5EEA59CF7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

// Was CMsgBoxIGSStorageItemInfo/CMsgBoxIGSStorageItemInfoLayout -- ported onto
// CGenericConfirmDialog (docs/rmlui-ui-system/dialog-migration-plan.md, "GameShop/MsgBoxIGS*.h").
// First non-C3DItemCommonMsgBox consumer of GenericDialogConfig::item3D -- see this .cpp's own
// comment for how a minimal ITEM snapshot is built from a bare item code.
void ShowIGSStorageItemInfoDialog(int iStorageSeq, int iStorageItemSeq, WORD wItemCode,
                                   char szItemType, const wchar_t* pszName, const wchar_t* pszNum,
                                   const wchar_t* pszPeriod);

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#endif // !defined(AFX_MSGBOXIGSUSEITEM_H__5E717B05_9D6D_4E85_B168_47D5EEA59CF7__INCLUDED_)
