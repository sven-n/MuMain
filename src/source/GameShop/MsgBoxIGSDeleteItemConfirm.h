// MsgBoxIGSDeleteItemConfirm.h: interface for the CMgsBoxIGSDeleteItemConfirm class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGBOXIGSDELETEITEMCONFIRM_H__96E35556_24A7_4A3B_86C3_D1ABF002AD99__INCLUDED_)
#define AFX_MSGBOXIGSDELETEITEMCONFIRM_H__96E35556_24A7_4A3B_86C3_D1ABF002AD99__INCLUDED_

#pragma once

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "UIControls.h"
#include "NewUIMessageBox.h"
#include "NewUICommonMessageBox.h"
#include "NewUIOptionWindow.h"

using namespace SEASON3B;

class CMsgBoxIGSDeleteItemConfirm : public CNewUIMessageBoxBase
{
public:
    enum IMAGE_IGS_DEL_ITEM_CONFIRM
    {
        IMAGE_IGS_BUTTON = BITMAP_IGS_MSGBOX_BUTTON,
        IMAGE_IGS_BACK = CNewUIOptionWindow::IMAGE_OPTION_FRAME_BACK,
        IMAGE_IGS_UP = CNewUIOptionWindow::IMAGE_OPTION_FRAME_UP,
        IMAGE_IGS_DOWN = CNewUIOptionWindow::IMAGE_OPTION_FRAME_DOWN,
        IMAGE_IGS_LEFTLINE = CNewUIOptionWindow::IMAGE_OPTION_FRAME_LEFT,
        IMAGE_IGS_RIGHTLINE = CNewUIOptionWindow::IMAGE_OPTION_FRAME_RIGHT,
    };

    enum IMAGESIZE_IGS_DEL_ITEM_CONFIRM
    {
        IMAGE_IGS_WINDOW_WIDTH = 640,
        IMAGE_IGS_WINDOW_HEIGHT = 429,
        IMAGE_IGS_FRAME_WIDTH = 190,
        IMAGE_IGS_FRAME_HEIGHT = 149,
        IMAGE_IGS_UP_HEIGHT = 64,
        IMAGE_IGS_DOWN_HEIGHT = 45,
        IMAGE_IGS_LINE_WIDTH = 21,
        IMAGE_IGS_LINE_HEIGHT = 10,
        IMAGE_IGS_BTN_WIDTH = 52,
        IMAGE_IGS_BTN_HEIGHT = 26,
    };

    enum IGS_DEL_ITEM_CONFIRM_POS
    {
        IGS_BTN_DEL_POS_X = 33,
        IGS_BTN_CANCEL_POS_X = 105,
        IGS_BTN_POS_Y = 110,
        IGS_TEXT_TITLE_Y = 10,
        IGS_TEXT_DIVIDE_WIDTH = 150,
        IGS_TEXT_DESCRIPTION_POS_X = 20,
        IGS_TEXT_DESCRIPTION_POS_Y = 50,
        IGS_TEXT_DESCRIPTION_INTERVAL = 12,
        IGS_TEXT_DESCRIPTION_WIDTH = 170,
    };

public:
    CMsgBoxIGSDeleteItemConfirm();
    ~CMsgBoxIGSDeleteItemConfirm();
    bool Create(float fPriority = 3.f);
    void Release();
    bool Update();
    bool Render();
    void Initialize(int iStorageSeq, int iStorageItemSeq, wchar_t szItemType);
    static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT CancelButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
private:
    void SetAddCallbackFunc();
    void SetButtonInfo();
    void RenderFrame();
    void RenderTexts();
    void RenderButtons();
    void LoadImages();
    void UnloadImages();
private:
    CNewUIMessageBoxButton m_BtnDelete;
    CNewUIMessageBoxButton m_BtnCancel;
    int		m_iMiddleCount;
    int		m_iStorageSeq;
    int		m_iStorageItemSeq;

    wchar_t m_szItemType;
    wchar_t m_szDescription[UIMAX_TEXT_LINE][MAX_TEXT_LENGTH];

    int			m_iDesciptionLine;
};

class CMsgBoxIGSDeleteItemConfirmLayout : public TMsgBoxLayout<CMsgBoxIGSDeleteItemConfirm>
{
public:
    bool SetLayout();
};

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#endif // !defined(AFX_MSGBOXIGSDELETEITEMCONFIRM_H__96E35556_24A7_4A3B_86C3_D1ABF002AD99__INCLUDED_)
