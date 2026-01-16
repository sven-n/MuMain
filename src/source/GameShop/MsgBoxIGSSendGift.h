// MsgBoxIGSSendGift.h: interface for the CMsgBoxIGSSendGift class.
//////////////////////////////////////////////////////////////////////

#pragma once
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "UIControls.h"
#include "NewUIMessageBox.h"
#include "NewUICommonMessageBox.h"

using namespace SEASON3B;

class CMsgBoxIGSSendGift : public CNewUIMessageBoxBase
{
public:
    enum IMAGE_IGS_SEND_GIFT
    {
        IMAGE_IGS_BUTTON = BITMAP_IGS_MSGBOX_BUTTON,
        IMAGE_IGS_FRAME = BITMAP_IGS_MSGBOX_SEND_GIFT_FRAME,		// Frame
        IMAGE_IGS_DECO = BITMAP_IGS_MSGBOX_SEND_GIFT_DECO,			// Deco
        IMAGE_IGS_INPUTTEXT = BITMAP_IGS_MSGBOX_SEND_GIFT_INPUTTEXT,	// Input TextBox
    };

    enum IMAGESIZE_IGS_SEND_GIFT
    {
        IMAGE_IGS_WINDOW_WIDTH = 640,	// 인게임샵 배경 사이즈
        IMAGE_IGS_WINDOW_HEIGHT = 429,
        IMAGE_IGS_FRAME_WIDTH = 210,	// 메세지박스 Size
        IMAGE_IGS_FRAME_HEIGHT = 267,
        IMAGE_IGS_DECO_WIDTH = 17,	// Deco
        IMAGE_IGS_DECO_HEIGHT = 19,
        IMAGE_IGS_ID_INPUT_BOX_WIDTH = 76,	// Input TextBox
        IMAGE_IGS_ID_INPUT_BOX_HEIGHT = 17,
        IMAGE_IGS_BTN_WIDTH = 52,	// 버튼 Size
        IMAGE_IGS_BTN_HEIGHT = 26,
    };

    // 메세지박스상의 상대좌표
    enum IGS_SEND_GIFT_POS
    {
        IMAGE_IGS_DECO_POS_X = 10,	// Deco
        IMAGE_IGS_DECO_POS_Y = 82,
        IMAGE_IGS_ID_INPUT_BOX_POS_X = 118,	// ID Input Box
        IMAGE_IGS_ID_INPUT_BOX_POS_Y = 82,
        IGS_ID_INPUT_TEXT_POS_X = 120,	// ID Input Text
        IGS_ID_INPUT_TEXT_POS_Y = 87,
        IGS_ID_INPUT_TEXT_WIDTH = 200,
        IGS_ID_INPUT_TEXT_HEIGHT = 14,
        IGS_MESSAGE_INPUT_TEXT_POS_X = 22,	// Message Input Text
        IGS_MESSAGE_INPUT_TEXT_POS_Y = 126,
        IGS_MESSAGE_INPUT_TEXT_WIDTH = 170,
        IGS_MESSAGE_INPUT_TEXT_HEIGHT = 65,
        IGS_MESSAGE_INPUT_TEXT_LINE_HEIGHT = 50,
        IGS_BTN_OK_POS_X = 35,
        IGS_BTN_CANCEL_POS_X = 122,
        IGS_BTN_POS_Y = 230,
        IGS_TEXT_TITLE_POS_Y = 10,	// Title
        IGS_TEXT_ID_TITLE_POS_X = 28,	// ID Title
        IGS_TEXT_ID_TITLE_POS_Y = 87,
        IGS_TEXT_ID_TITLE_WIDTH = 100,
        IGS_TEXT_MESSAGE_TITLE_POS_Y = 110,	// Message Title
        IGS_TEXT_ITEM_INFO_POS_X = 30,	// ItemInfo
        IGS_TEXT_ITEM_INFO_NAME_POS_Y = 38,
        IGS_TEXT_ITEM_INFO_PRICE_POS_Y = 50,
        IGS_TEXT_ITEM_INFO_PERIOD_POS_Y = 62,
        IGS_TEXT_ITEM_INFO_WIDTH = 143,
        IGS_TEXT_NOTICE_POS_Y = 205,	// Notice
        IGS_TEXT_NOTICE_WIDTH = 170,
    };

public:
    CMsgBoxIGSSendGift();
    ~CMsgBoxIGSSendGift();

    bool Create(float fPriority = 3.f);
    void Release();

    bool Update();
    bool Render();

    void Initialize(int iPackageSeq, int iDisplaySeq, int iPriceSeq, DWORD wItemCode, int iCashType, wchar_t* pszName, wchar_t* pszPrice, wchar_t* pszPeriod);

    static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT CancelButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

private:
    void SetAddCallbackFunc();
    void SetButtonInfo();

    void RenderFrame();
    void RenderTexts();
    void RenderButtons();

    void ChangeInputBoxFocus();

    void LoadImages();
    void UnloadImages();

    void InitInputBox();

private:
    // buttons
    CNewUIMessageBoxButton m_BtnOk;
    CNewUIMessageBoxButton m_BtnCancel;

    CUITextInputBox		m_IDInputBox;
    CUITextInputBox		m_MessageInputBox;

    int		m_iPackageSeq;
    int		m_iDisplaySeq;
    int		m_iPriceSeq;
    DWORD	m_wItemCode;
    int		m_iCashType;

    wchar_t m_szID[MAX_ID_SIZE + 1];
    wchar_t m_szMessage[MAX_GIFT_MESSAGE_SIZE];

    wchar_t m_szName[MAX_TEXT_LENGTH];
    wchar_t m_szPrice[MAX_TEXT_LENGTH];
    wchar_t m_szPeriod[MAX_TEXT_LENGTH];

    wchar_t m_szNotice[NUM_LINE_CMB][MAX_TEXT_LENGTH];

    int		m_iNumNoticeLine;
};

class CMsgBoxIGSSendGiftLayout : public TMsgBoxLayout<CMsgBoxIGSSendGift>
{
public:
    bool SetLayout();
};

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
