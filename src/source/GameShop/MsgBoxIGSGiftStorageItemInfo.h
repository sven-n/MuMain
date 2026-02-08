// MsgBoxIGSGiftStorageItemInfo.h: interface for the CMsgBoxIGSGiftStorageItemInfo class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGBOXIGSGIFTSTORAGEITEMINFO_H__C2B5CD4A_A0D5_4F4D_854B_C9119EE0EED6__INCLUDED_)
#define AFX_MSGBOXIGSGIFTSTORAGEITEMINFO_H__C2B5CD4A_A0D5_4F4D_854B_C9119EE0EED6__INCLUDED_

#pragma once

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "UIControls.h"
#include "NewUIMessageBox.h"
#include "NewUICommonMessageBox.h"

using namespace SEASON3B;

class CMsgBoxIGSGiftStorageItemInfo : public CNewUIMessageBoxBase, public INewUI3DRenderObj
{
public:
    enum IMAGE_IGS_GIFT_STORAGE_ITEM_INFO
    {
        IMAGE_IGS_BUTTON = BITMAP_IGS_MSGBOX_BUTTON,
        IMAGE_IGS_FRAME = BITMAP_IGS_MSGBOX_GIFT_STORAGE_ITEM,
    };

    enum IMAGESIZE_IGS_GIFT_STORAGE_ITEM_INFO
    {
        IMAGE_IGS_WINDOW_WIDTH = 640,
        IMAGE_IGS_WINDOW_HEIGHT = 429,
        IMAGE_IGS_FRAME_WIDTH = 210,
        IMAGE_IGS_FRAME_HEIGHT = 306,
        IMAGE_IGS_BTN_WIDTH = 52,
        IMAGE_IGS_BTN_HEIGHT = 26,
    };

    enum IGS_GIFT_STORAGE_ITEM_INFO_POS
    {
        IGS_BTN_OK_POS_X = 43,
        IGS_BTN_CANCEL_POS_X = 115,
        IGS_BTN_POS_Y = 168,
        IGS_TEXT_TITLE_POS_Y = 10,
        IGS_TEXT_ITEM_NAME_POS_Y = 100,
        IGS_TEXT_ITEM_INFO_POS_X = 30,
        IGS_TEXT_ITEM_INFO_NUM_POS_Y = 124,
        IGS_TEXT_ITEM_INFO_PERIOD_POS_Y = 140,
        IGS_TEXT_ITEM_INFO_WIDTH = 150,
        IGS_TEXT_ID_INFO_POS_X = 30,
        IGS_TEXT_ID_INFO_POS_Y = 177,
        IGS_TEXT_ID_INFO_WIDTH = 150,
        IGS_MESSAGE_INPUT_TEXT_POS_X = 126,
        IGS_MESSAGE_INPUT_TEXT_POS_Y = 96,
        IGS_3DITEM_POS_X = 56,
        IGS_3DITEM_POS_Y = 34,
        IGS_3DITEM_WIDTH = 97,
        IGS_3DITEM_HEIGHT = 60,
    };

public:
    CMsgBoxIGSGiftStorageItemInfo();
    ~CMsgBoxIGSGiftStorageItemInfo();
    bool Create(float fPriority = 3.f);
    void Release();
    bool Update();
    bool Render();
    bool IsVisible() const;
    void Render3D();
    void Initialize(int iStorageSeq, int iStorageItemSeq, WORD wItemCode, wchar_t szItemType, wchar_t* pszID, wchar_t* pszMessage, wchar_t* pszName, wchar_t* pszNum, wchar_t* pszPeriod);
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
    CNewUIMessageBoxButton m_BtnUse;
    CNewUIMessageBoxButton m_BtnCancel;
    CUITextInputBox		m_MessageInputBox;

    int		m_iStorageSeq;
    int		m_iStorageItemSeq;
    WORD	m_wItemCode;

    wchar_t	m_szName[MAX_TEXT_LENGTH];
    wchar_t	m_szNum[MAX_TEXT_LENGTH];
    wchar_t m_szPeriod[MAX_TEXT_LENGTH];
    char m_szItemType;
    wchar_t	m_szIDInfo[MAX_TEXT_LENGTH];
    wchar_t	m_szMessage[MAX_GIFT_MESSAGE_SIZE];
};

class CMsgBoxIGSGiftStorageItemInfoLayout : public TMsgBoxLayout<CMsgBoxIGSGiftStorageItemInfo>
{
public:
    bool SetLayout();
};

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#endif // !defined(AFX_MSGBOXIGSGIFTSTORAGEITEMINFO_H__C2B5CD4A_A0D5_4F4D_854B_C9119EE0EED6__INCLUDED_)
