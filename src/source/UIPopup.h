//  UIPopup.h
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIPOPUP_H__1FD0B6C0_1EB3_4805_965C_C53A6A9A39B3__INCLUDED_)
#define AFX_UIPOPUP_H__1FD0B6C0_1EB3_4805_965C_C53A6A9A39B3__INCLUDED_

#pragma once

#define	POPUP_CUSTOM	0
#define	POPUP_OK		1
#define	POPUP_OKCANCEL	2
#define	POPUP_YESNO		4
#define	POPUP_TIMEOUT	8
#define	POPUP_INPUT		16

typedef int POPUP_RESULT;
#define	POPUP_RESULT_NONE		1
#define	POPUP_RESULT_OK			2
#define	POPUP_RESULT_CANCEL		4
#define	POPUP_RESULT_YES		8
#define	POPUP_RESULT_NO			16
#define	POPUP_RESULT_TIMEOUT	32
#define	POPUP_RESULT_ESC		64

enum POPUP_ALIGN
{
    PA_CENTER,
    PA_TOP
};

#define MAX_POPUP_TEXTLINE		10
#define MAX_POPUP_TEXTLENGTH	256

class CUIPopup
{
public:
    CUIPopup();
    virtual ~CUIPopup();

protected:
    int (*PopupResultFuncPointer)(POPUP_RESULT Result);
    void (*PopupUpdateInputFuncPointer)();
    void (*PopupRenderFuncPointer)();

    DWORD			m_dwPopupID;
    int				m_nPopupTextCount;
    wchar_t			m_szPopupText[MAX_POPUP_TEXTLINE][MAX_POPUP_TEXTLENGTH];
    int				m_PopupType;
    SIZE			m_sizePopup;
    POPUP_ALIGN		m_Align;

    wchar_t			m_szInputText[1024];
    int				m_nInputSize;
    int				m_nInputTextLength;
    UIOPTIONS		m_InputOptions;

    DWORD			m_dwPopupStartTime;
    DWORD			m_dwPopupEndTime;
    DWORD			m_dwPopupElapseTime;

    CUIButton		m_OkButton;
    CUIButton		m_CancelButton;
    CUIButton		m_YesButton;
    CUIButton		m_NoButton;

protected:
    bool CheckTimeOut();

public:
    void Init();
    DWORD SetPopup(const wchar_t* pszText, int nLineCount, int nBufferSize, int Type, int (*ResultFunc)(POPUP_RESULT Result), POPUP_ALIGN Align = PA_CENTER);
    void SetPopupExtraFunc(void (*InputFunc)(), void (*RenderFunc)());
    void SetInputMode(int nSize, int nTextLength, UIOPTIONS Options);
    bool IsInputEnable();
    void SetTimeOut(DWORD dwElapseTime);
    wchar_t* GetInputText();
    DWORD GetPopupID();
    void Close();
    void CancelPopup();
    bool PressKey(int nKey);
    void UpdateInput();
    void Render();
};

#endif // !defined(AFX_UIPOPUP_H__1FD0B6C0_1EB3_4805_965C_C53A6A9A39B3__INCLUDED_)
