//  UIPopup.h
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIPOPUP_H__1FD0B6C0_1EB3_4805_965C_C53A6A9A39B3__INCLUDED_)
#define AFX_UIPOPUP_H__1FD0B6C0_1EB3_4805_965C_C53A6A9A39B3__INCLUDED_

#pragma once

// Every built-in popup shape (OK/OK-Cancel/Yes-No/timeout/text-input) has been migrated to
// CGenericConfirmDialog/CGenericMenuDialog. The one remaining caller (Guild appoint-role
// confirm, UIGuildInfo.cpp) draws its own layout entirely through SetPopupExtraFunc(), so this
// class is now just an open/close handle plus an optional Escape-cancel callback.
typedef int POPUP_RESULT;
#define POPUP_RESULT_NONE 1
#define POPUP_RESULT_ESC 64

class CUIPopup
{
public:
    CUIPopup();
    virtual ~CUIPopup();

protected:
    int (*PopupResultFuncPointer)(POPUP_RESULT Result);
    void (*PopupUpdateInputFuncPointer)();
    void (*PopupRenderFuncPointer)();

    DWORD m_dwPopupID;

public:
    void Init();
    DWORD SetPopup(int (*ResultFunc)(POPUP_RESULT Result));
    void SetPopupExtraFunc(void (*InputFunc)(), void (*RenderFunc)());
    DWORD GetPopupID();
    void Close();
    void CancelPopup();
    bool PressKey(int nKey);
    void UpdateInput();
    void Render();
};

#endif // !defined(AFX_UIPOPUP_H__1FD0B6C0_1EB3_4805_965C_C53A6A9A39B3__INCLUDED_)
