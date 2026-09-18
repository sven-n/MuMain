#pragma once
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

// Was CMsgBoxIGSSendGiftConfirm/CMsgBoxIGSSendGiftConfirmLayout -- ported onto
// CGenericConfirmDialog (docs/rmlui-ui-system/dialog-migration-plan.md, "GameShop/MsgBoxIGS*.h").
// Kept as a free function in this file since CMsgBoxIGSSendGift.cpp -- staying native (needs two
// simultaneous text-entry fields, a genuine GenericDialogConfig gap) -- still needs to open this
// confirm dialog once its own recipient-ID/message validation passes.
void ShowIGSSendGiftConfirmDialog(int iPackageSeq, int iDisplaySeq, int iPriceSeq, DWORD wItemCode,
                                   int iCashType, std::wstring id, std::wstring message,
                                   const wchar_t* pszName, const wchar_t* pszPrice,
                                   const wchar_t* pszPeriod);

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
