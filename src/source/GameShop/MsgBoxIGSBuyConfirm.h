
#if !defined(AFX_MSGBOXIGSBUYCONFIRM_H__7C4F8295_EA55_4F9C_A7C3_B514C11BE83C__INCLUDED_)
#define AFX_MSGBOXIGSBUYCONFIRM_H__7C4F8295_EA55_4F9C_A7C3_B514C11BE83C__INCLUDED_

#pragma once

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

// Was CMsgBoxIGSBuyConfirm/CMsgBoxIGSBuyConfirmLayout -- ported onto CGenericConfirmDialog
// (docs/rmlui-ui-system/dialog-migration-plan.md, "GameShop/MsgBoxIGS*.h"). Kept as a free function
// in this file (not folded into the caller) since CMsgBoxIGSBuyPackageItem.cpp/
// CMsgBoxIGSBuySelectItem.cpp -- both staying native (genuine Buy/Present/Cancel 3-button + list-box
// shape CGenericConfirmDialog can't reproduce) -- still need to open this confirm dialog from their
// own Buy button.
void ShowIGSBuyConfirmDialog(int iPackageSeq, int iDisplaySeq, int iPriceSeq, WORD wItemCode,
                             int iCashType, const wchar_t* pszName, const wchar_t* pszPrice,
                             const wchar_t* pszPeriod);

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#endif // !defined(AFX_MSGBOXIGSBUYCONFIRM_H__7C4F8295_EA55_4F9C_A7C3_B514C11BE83C__INCLUDED_)
