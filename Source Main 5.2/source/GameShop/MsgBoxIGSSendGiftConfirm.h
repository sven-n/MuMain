// MsgBoxIGSSendGiftConfirm.h: interface for the CMsgBoxIGSSendGiftConfirm class.
//////////////////////////////////////////////////////////////////////
#pragma once
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "UIControls.h"
#include "NewUIOptionWindow.h"
#include "NewUIMessageBox.h"
#include "NewUICommonMessageBox.h"

using namespace SEASON3B;

class CMsgBoxIGSSendGiftConfirm : public CNewUIMessageBoxBase
{
public:
    enum IMAGE_IGS_SEND_GIFT_CONFIRM
    {
        IMAGE_IGS_BUTTON = BITMAP_IGS_MSGBOX_BUTTON,
        IMAGE_IGS_BACK = CNewUIOptionWindow::IMAGE_OPTION_FRAME_BACK,
        IMAGE_IGS_UP = CNewUIOptionWindow::IMAGE_OPTION_FRAME_UP,
        IMAGE_IGS_DOWN = CNewUIOptionWindow::IMAGE_OPTION_FRAME_DOWN,
        IMAGE_IGS_LEFTLINE = CNewUIOptionWindow::IMAGE_OPTION_FRAME_LEFT,
        IMAGE_IGS_RIGHTLINE = CNewUIOptionWindow::IMAGE_OPTION_FRAME_RIGHT,
        IMAGE_IGS_TEXTBOX = BITMAP_IGS_MGSBOX_BUY_CONFIRM_TEXT_BOX,
    };

    enum IMAGESIZE_IGS_SEND_GIFT_CONFIRM
    {
        IMAGE_IGS_WINDOW_WIDTH = 640,
        IMAGE_IGS_WINDOW_HEIGHT = 429,
        IMAGE_IGS_FRAME_WIDTH = 190,
        IMAGE_IGS_FRAME_HEIGHT = 179,
        IMAGE_IGS_TEXTBOX_WIDTH = 160,
        IMAGE_IGS_TEXTBOX_HEIGHT = 41,
        IMAGE_IGS_UP_HEIGHT = 64,
        IMAGE_IGS_DOWN_HEIGHT = 45,
        IMAGE_IGS_LINE_WIDTH = 21,
        IMAGE_IGS_LINE_HEIGHT = 10,
        IMAGE_IGS_BTN_WIDTH = 52,
        IMAGE_IGS_BTN_HEIGHT = 26,
    };

    enum IGS_SEND_GIFT_CONFIRM_POS
    {
        IGS_BTN_OK_POS_X = 35,
        IGS_BTN_CANCEL_POS_X = 105,
        IGS_BTN_POS_Y = 140,
        IGS_TEXTBOX_POS_X = 15,
        IGS_TEXTBOX_POS_Y = 58,
        IGS_TEXT_TITLE_POS_Y = 10,
        IGS_TEXT_QUESTION_POS_Y = 42,
        IGS_TEXT_NOTICE_POS_Y = 105,
        IGS_TEXT_NOTICE_WIDTH = 150,
        IGS_TEXT_ITEM_INFO_POS_X = 25,
        IGS_TEXT_ITEM_INFO_NAME_POS_Y = 64,
        IGS_TEXT_ITEM_INFO_PRICE_POS_Y = 75,
        IGS_TEXT_ITEM_INFO_PERIOD_POS_Y = 86,
        IGS_TEXT_ITEM_INFO_WIDTH = 143,
    };

public:
    CMsgBoxIGSSendGiftConfirm();
    ~CMsgBoxIGSSendGiftConfirm();

    bool Create(float fPriority = 3.f);
    void Release();

    bool Update();
    bool Render();

    void CMsgBoxIGSSendGiftConfirm::Initialize(int iPackageSeq, int iDisplaySeq, int iPriceSeq, DWORD wItemCode, int iCashType, wchar_t* pszID, wchar_t* pszMessage, wchar_t* pszName, wchar_t* pszPrice, wchar_t* pszPeriod);

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
    // buttons
    CNewUIMessageBoxButton m_BtnOk;
    CNewUIMessageBoxButton m_BtnCancel;

    int m_iMiddleCount;

    int		m_iPackageSeq;
    int		m_iDisplaySeq;
    int		m_iPriceSeq;
    DWORD	m_wItemCode;
    int		m_iCashType;

    wchar_t m_szID[MAX_ID_SIZE + 1];
    wchar_t m_szMessage[MAX_GIFT_MESSAGE_SIZE];

    wchar_t m_szItemName[MAX_TEXT_LENGTH];
    wchar_t m_szItemPrice[MAX_TEXT_LENGTH];
    wchar_t m_szItemPeriod[MAX_TEXT_LENGTH];

    wchar_t m_szNotice[NUM_LINE_CMB][MAX_TEXT_LENGTH];

    int		m_iNumNoticeLine;
};

class CMsgBoxIGSSendGiftConfirmLayout : public TMsgBoxLayout<CMsgBoxIGSSendGiftConfirm>
{
public:
    bool SetLayout();
};

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
